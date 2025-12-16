#pragma once
#include <cstdint>
#include <vector>

class I2CDevice {
public:
    I2CDevice(int bus = 1, uint8_t addr = 0x68);
    ~I2CDevice();
    void setAddress(uint8_t addr);

    void writeByte(uint8_t reg, uint8_t val);
    void writeBytes(uint8_t reg, const std::vector<uint8_t>& data);
    uint8_t readByte(uint8_t reg);
    std::vector<uint8_t> readBytes(uint8_t reg, size_t len);

    void writeRawByte(uint8_t b);
    int  readRawByte();

private:
    int fd_;
    int bus_;
    uint8_t addr_;
    void openBus();
};
