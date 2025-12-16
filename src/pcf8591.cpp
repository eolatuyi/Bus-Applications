#include "pcf8591.hpp"
#include <stdexcept>

uint8_t PCF8591::readAnalog(uint8_t ch) {
    if (ch > 3) throw std::invalid_argument("PCF8591 ch out of range");
    uint8_t ctrl = 0x40 | (ch & 0x03);
    dev_.writeRawByte(ctrl);
    (void)dev_.readRawByte();
    int v = dev_.readRawByte();
    if (v < 0) throw std::runtime_error("PCF8591 read failed");
    return static_cast<uint8_t>(v);
}

void PCF8591::writeDAC(uint8_t value) {
    uint8_t ctrl = 0x40 | 0x40;
    dev_.writeRawByte(ctrl);
    dev_.writeRawByte(value);
}
