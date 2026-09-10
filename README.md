# Multi-Bus Sensor Dashboard (Raspberry Pi)

**Buses covered:** I2C (MPU6050 + ADS7830), SPI (74HC595), GPIO (LCD1602 4-bit)

## Hardware Topology

- **I2C** `/dev/i2c-1` (3.3V)
  - **MPU6050: Accelerometer/gyro/Temp** @ `0x68` (AD0=LOW - address wiring config)
  - **ADS7830: Potentiometer (8-bit ADC)** @ `0x4B` (Freenove module; A0/A1 high, fixed address)
    - Potentiometer input on **CH2** (Freenove projects board wiring)
- **SPI: Output expander for LED array** `/dev/spidev0.0`
  - **74HC595**: one 8-bit chip (not three). Datasheet “3-state” means Q0–Q7 can be High / Low / Hi-Z via `OE`; internally it is an 8-stage shift register plus an 8-bit latch.
  - **SPI map**: SER=MOSI (GPIO10), SRCLK=SCLK (GPIO11), RCLK=CE0 (GPIO8)
  - Tie **OE to GND** (outputs enabled) and **SRCLR / MR to 3.3V** (do not clear).
  - SparkFun 10-segment bar: use 8 segments. **Anodes → Q0–Q7**, **cathodes → 220Ω → GND**. If resistors are on the ground side and nothing lights, rotate the bar 180°.
- **LCD1602 (parallel, 4-bit): LCD connection** via **GPIO** using **libgpiod**
  - Default pin mapping (BCM): `RS=17`, `E=27`, `D4=22`, `D5=23`, `D6=24`, `D7=25`
  - RW → GND (write-only), VCC → 5V, GND → GND, VO → contrast pot (approx 0.3–0.6V)

## Wiring Diagram (as built)

Not wired yet: Arduino UNO.

![Hardware topology](docs/HL-diagram.svg)

GitHub inline preview (Mermaid):

```mermaid
flowchart TB
  Pi["Raspberry Pi 3 Model B"]

  subgraph i2cBus ["I2C SDA GPIO2 SCL GPIO3"]
    MPU["MPU6050 0x68"]
    ADC["ADS7830 0x4B pot CH2"]
  end

  subgraph spiBus ["SPI0 MOSI10 SCLK11 CE0"]
    SR["74HC595"]
    BAR["SparkFun bar 8 of 10"]
    R["220 ohm to GND"]
    SR --> BAR --> R
  end

  Pi --> MPU
  Pi --> ADC
  Pi --> SR
```

Editable SVG (IDE / browser): [`docs/HL-diagram.svg`](docs/HL-diagram.svg)

## Software Setup

```bash
sudo raspi-config            # Enable I2C and SPI
sudo apt update
sudo apt install -y build-essential cmake git libi2c-dev i2c-tools libgpiod-dev
```

## Build & Run

```bash
mkdir build && cd build
cmake ..
make
ctest --output-on-failure   # host-side unit tests (protocol, CLI, LED bar math)
./app --test-hc595          # SPI only: walk Q0-Q7 then fill LED bar (no I2C/LCD/pot)
./app --test-lcd            # GPIO only: HD44780 bring-up (no I2C/SPI). Needs gpio group.
./app --no-lcd              # I2C + SPI only while LCD is unwired (no sudo if in i2c/spi groups)
./app                       # full dashboard once LCD is wired (prefer without sudo if in gpio group)
```

`--no-lcd`, `--test-hc595`, and `--test-lcd` are mutually exclusive. The pot maps
onto all eight LED-bar segments (`analog 255` → Q0–Q7 on).

### Hardware-in-the-loop (on the Pi)

```bash
chmod +x scripts/hil_test.sh
./scripts/hil_test.sh                  # smoke: i2cdetect + SPI/LCD bring-up + --no-lcd + 3 s ./app
STRICT_POT=1 ./scripts/hil_test.sh     # also fail if pot not moved during capture
STRICT_LCD=1 ./scripts/hil_test.sh     # fail if --test-lcd / full ./app cannot open GPIO
```

## Cursor / review process

This repo consumes skills from the personal `sdlc-skills` collection.

- Review rule: `.cursor/rules/embedded-code-review.mdc` (adapter; canonical
  source lives in `sdlc-skills`)
- Architecture discovery: `.cursor/rules/architectural-discovery.mdc`
  (adapter; canonical source + `scripts/architect.py` live in `sdlc-skills`)
- Repo defaults: `.cursor/rules/project-context.mdc`
- Architecture: `docs/architecture.md`
- Local reviews only: `reviews/<path-mirroring-source>/<YYYY-MM-DD>_<short-hash>.md`
  (gitignored; do not commit or push)

Open this directory as the Cursor workspace so those rules load.

## Test milestones (current project status)

- Initial build on Raspberry Pi 3 Model B Rev 1.2: **Completed** — `app` builds on target (confirmed).
- Test on target device (functional/system testing): **In progress**
  - **MPU6050** @ `0x68`: **Verified** — accel/gyro/temp readings sane on hardware (`--no-lcd` run); `init()` checks `WHO_AM_I`.
  - **ADS7830** @ `0x4B` (CH2 pot): **Verified** — `Pot=` tracks knob; HIL smoke + operator confirm.
  - **74HC595** / SPI LED bar: **Verified** — walk/bar via `--test-hc595`; bar tracks pot under `./app --no-lcd`.
  - **LCD1602** / GPIO: **Verified** — `--test-lcd` bring-up on hardware; line 1 `LCD bring-up OK`, line 2 counting; GPIO 17/27/22–25 claimed by `LCD1602`. Full `./app` shows live `Pot:` / `ax:` on the panel (rows padded to 16 columns).
- Unit testing: **In progress** — `ads7830_protocol_test`, `hc595_bar_test`, `mpu6050_protocol_test`, `lcd1602_protocol_test`, `app_cli_test` via `ctest`.
- System testing / integration: **Partial** — MPU6050 + ADS7830 + 74HC595 + LED bar + LCD dashboard verified on hardware (`./app`). HIL on Pi: `i2cdetect`, `--test-hc595`, `--test-lcd`, `--no-lcd`, full `./app` (LCD on). Arduino UNO still unwired.

