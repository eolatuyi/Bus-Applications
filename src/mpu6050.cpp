#include "mpu6050.hpp"

static constexpr uint8_t REG_PWR_MGMT_1 = 0x6B;
static constexpr uint8_t REG_ACCEL_XOUT_H = 0x3B;

static constexpr float ACCEL_SCALE = 16384.0f;
static constexpr float GYRO_SCALE  = 131.0f;

MPU6050::MPU6050(uint8_t addr) : dev_(1, addr) {}

void MPU6050::init() {
    dev_.writeByte(REG_PWR_MGMT_1, 0x00);
}

MpuRead MPU6050::read() {
    auto d = dev_.readBytes(REG_ACCEL_XOUT_H, 14);
    auto s16 = [&](int idx){ return (int16_t)((d[idx]<<8) | d[idx+1]); };
    int16_t ax = s16(0), ay = s16(2), az = s16(4);
    int16_t temp = s16(6);
    int16_t gx = s16(8), gy = s16(10), gz = s16(12);

    MpuRead r;
    r.ax_g = ax / ACCEL_SCALE;
    r.ay_g = ay / ACCEL_SCALE;
    r.az_g = az / ACCEL_SCALE;
    r.gx_dps = gx / GYRO_SCALE;
    r.gy_dps = gy / GYRO_SCALE;
    r.gz_dps = gz / GYRO_SCALE;
    r.temp_c = (temp / 340.0f) + 36.53f;
    return r;
}
