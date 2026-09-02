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
  /dev/i2c-1   /dev/spidev0.0  gpiochip0
```

## Layers

| Layer | Files | Responsibility | Must not |
|-------|--------|----------------|----------|
| Application | `src/app.cpp` | Own the devices, 200 ms poll loop, stdout + LCD presentation, map analog → LED bar | Open `/dev/*` or GPIO chips; encode register maps |
| Device drivers | `src/mpu6050.*`, `src/ads7830.*`, `src/ads7830_protocol.hpp`, `src/hc595.*`, `src/hc595_bar.hpp`, `src/lcd1602.*` | Device protocol, init, scaling to engineering units | Dashboard copy, polling policy, other devices' protocols |
| Bus wrappers | `src/i2c_device.*`, `src/spi_device.*` | Linux I2C/SPI file descriptors, ioctl, raw read/write | Device register names, scaling, UI |

LCD1602 talks GPIO directly (4-bit HD44780) rather than a bus wrapper.
That is the documented exception: parallel LCD is not on I2C/SPI.

## Runtime model

- One thread. Init throws on open/config failure; the main loop has no
  retry/backoff beyond process exit.
- Hardware topology and addresses: `README.md`.
- Reviews of C/C++ changes follow the `embedded-code-review` Cursor rule
  and may be written under `reviews/` locally; that directory is
  gitignored and is not pushed.
