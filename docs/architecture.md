# Architecture — Multi-Bus Sensor Dashboard

Linux userspace C++17 process on Raspberry Pi. Three layers; dependencies
point downward only.

```
src/app.cpp          application (poll, print, LCD text, LED bar)
        │
        ▼
MPU6050  ADS7830  HC595  LCD1602     device drivers (protocol + scaling)
        │            │         │
        ▼            ▼         ▼
   I2CDevice    SPIDevice   libgpiod    bus / GPIO wrappers
        │            │         │
        ▼            ▼         ▼
  /dev/i2c-1   /dev/spidev0.0  /dev/gpiochip0
```

## Layers

| Layer | Files | Responsibility | Must not |
|-------|--------|----------------|----------|
| Application | `src/app.cpp`, `src/app_cli.hpp` | Own the devices, 200 ms poll loop, stdout + LCD presentation, map analog → LED bar, CLI flags | Open `/dev/*` or GPIO chips; encode register maps |
| Device drivers | `src/mpu6050.*`, `src/mpu6050_protocol.hpp`, `src/ads7830.*`, `src/ads7830_protocol.hpp`, `src/hc595.*`, `src/hc595_bar.hpp`, `src/lcd1602.*`, `src/lcd1602_protocol.hpp` | Device protocol, init, scaling to engineering units | Dashboard copy, polling policy, other devices' protocols |
| Bus wrappers | `src/i2c_device.*`, `src/spi_device.*` | Linux I2C/SPI file descriptors, ioctl, raw read/write | Device register names, scaling, UI |

LCD1602 talks GPIO directly (4-bit HD44780) rather than a bus wrapper.
That is the documented exception: parallel LCD is not on I2C/SPI.

`I2CDevice` / `SPIDevice` / `LCD1602` are non-copyable and non-movable. Bus
constructors close the fd before throwing if a later ioctl fails. LCD requests
all six GPIO lines in one libgpiod v2 request so a failed ctor cannot leak
earlier line requests.

## Runtime model

- One thread. Init throws on open/config failure (MPU6050 also throws if
  `WHO_AM_I` is not `0x68`). The dashboard poll loop, `--test-hc595`, and
  `--test-lcd` retry transient I/O errors (50 ms) rather than exiting.
  `--no-lcd`, `--test-hc595`, and `--test-lcd` are mutually exclusive.
- Hardware topology and addresses: `README.md`.
- Reviews of C/C++ changes follow the `embedded-code-review` Cursor rule
  and may be written under `reviews/` locally; that directory is
  gitignored and is not pushed.
