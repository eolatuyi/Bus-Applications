# Multi-Bus Sensor Dashboard (Raspberry Pi)

**Buses covered:** I2C (MPU6050 + PCF8591), SPI (74HC595), GPIO (LCD1602 4-bit)

## Hardware Topology

- **I2C** `/dev/i2c-1` (3.3V)
  - **MPU6050: Accelerometer/gyro/Temp** @ `0x68` (AD0=LOW - address wiring config)
  - **PCF8591: Potentiometer** @ `0x48` (A0–A2=LOW - address wiring config)
- **SPI: Output expander for LED array** `/dev/spidev0.0`
  - **74HC595**: SER=MOSI (GPIO10), SRCLK=SCLK (GPIO11), RCLK=CE0 (GPIO8)
- **LCD1602 (parallel, 4-bit): LCD connection** via **GPIO** using **libgpiod**
  - Default pin mapping (BCM): `RS=17`, `E=27`, `D4=22`, `D5=23`, `D6=24`, `D7=25`
  - RW → GND (write-only), VCC → 5V, GND → GND, VO → contrast pot (approx 0.3–0.6V)

## Wiring Diagram

![Wiring Diagram](docs/HL-diagram.png)

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
sudo ./app
```

## Test milestones (current project status)

- Initial build on Raspberry Pi Model B 2 SoC: **Completed** — an initial `app` binary builds on the target Pi (confirmed).
- Test on target device (functional/system testing): **Not started** — recommend running the app on hardware and verifying I2C, SPI and GPIO peripherals.
- Unit testing: **Not started** — may be bypassed if system testing is straightforward and covers required flows.
- System testing / integration: **Planned** — exercise the full stack (MPU6050, PCF8591, 74HC595, LCD1602) and validate behavior under expected conditions.

