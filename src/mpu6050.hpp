#pragma once
#include "i2c_device.hpp"

struct MpuRead {
    float ax_g, ay_g, az_g;
    float gx_dps, gy_dps, gz_dps;
    float temp_c;
};

class MPU6050 {
public:
    explicit MPU6050(uint8_t addr = 0x68);
    void init();
    MpuRead read();
private:
    I2CDevice dev_;
};
