#pragma once
#include "i2c_device.hpp"
#include <cstdint>

class PCF8591 {
public:
    explicit PCF8591(uint8_t addr = 0x48) : dev_(1, addr) {}
    uint8_t readAnalog(uint8_t ch);
    void writeDAC(uint8_t value);
private:
    I2CDevice dev_;
};
