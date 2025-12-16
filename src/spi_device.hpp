#pragma once
#include <cstdint>
#include <vector>

class SPIDevice {
public:
    SPIDevice(const char* dev = "/dev/spidev0.0", uint32_t speed = 1'000'000);
    ~SPIDevice();
    void transfer(const std::vector<uint8_t>& tx);
private:
    int fd_;
};
