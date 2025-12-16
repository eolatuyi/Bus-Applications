#pragma once
#include "spi_device.hpp"
#include <cstdint>

class HC595 {
public:
    HC595() : spi_("/dev/spidev0.0", 1'000'000) {}
    void writeByte(uint8_t b) { spi_.transfer({b}); }
    void writeBarFromAnalog(uint8_t a0);
private:
    SPIDevice spi_;
};
