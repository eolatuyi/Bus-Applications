#pragma once
#include "i2c_device.hpp"
#include <cstdint>

class ADS7830 {
public:
    // Freenove ADS7830 module: A0/A1 tied high → 0x4B (fixed, not jumper-configurable).
    static constexpr uint8_t kAddr = 0x4B;
    // Freenove projects board routes the potentiometer to CH2.
    static constexpr uint8_t kPotChannel = 2;

    ADS7830() : dev_(1, kAddr) {}

    uint8_t readAnalog(uint8_t ch);

private:
    I2CDevice dev_;
};
