#pragma once
#include "i2c_device.hpp"
#include "mpu6050_protocol.hpp"

class MPU6050 {
public:
    static constexpr uint8_t kAddr = 0x68;

    explicit MPU6050(uint8_t addr = kAddr);
    void init();
    MpuRead read();
private:
    I2CDevice dev_;
};
