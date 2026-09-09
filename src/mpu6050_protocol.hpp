#pragma once
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

constexpr uint8_t kMpuRegPwrMgmt1 = 0x6B;
constexpr uint8_t kMpuRegAccelXoutH = 0x3B;
constexpr uint8_t kMpuRegWhoAmI = 0x75;
constexpr uint8_t kMpuWhoAmIValue = 0x68;

constexpr float kMpuAccelScale = 16384.0f;  // ±2 g
constexpr float kMpuGyroScale = 131.0f;     // ±250 dps

struct MpuRead {
    float ax_g, ay_g, az_g;
    float gx_dps, gy_dps, gz_dps;
    float temp_c;
};

inline int16_t mpu6050Be16(uint8_t hi, uint8_t lo) {
    return static_cast<int16_t>(
        (static_cast<uint16_t>(hi) << 8) | static_cast<uint16_t>(lo));
}

inline float mpu6050AccelG(int16_t raw) { return raw / kMpuAccelScale; }
inline float mpu6050GyroDps(int16_t raw) { return raw / kMpuGyroScale; }
inline float mpu6050TempC(int16_t raw) { return (raw / 340.0f) + 36.53f; }

inline bool mpu6050WhoAmIOk(uint8_t id) { return id == kMpuWhoAmIValue; }

inline bool mpu6050BurstLooksStuck(const std::vector<uint8_t>& d) {
    if (d.size() < 14) {
        return true;
    }
    for (size_t i = 0; i < 14; ++i) {
        if (d[i] != 0xFF) {
            return false;
        }
    }
    return true;
}

inline MpuRead mpu6050Decode(const std::vector<uint8_t>& d) {
    if (d.size() < 14) {
        throw std::invalid_argument("MPU6050 burst shorter than 14 bytes");
    }
    if (mpu6050BurstLooksStuck(d)) {
        throw std::runtime_error("MPU6050 burst stuck at 0xFF");
    }
    const int16_t ax = mpu6050Be16(d[0], d[1]);
    const int16_t ay = mpu6050Be16(d[2], d[3]);
    const int16_t az = mpu6050Be16(d[4], d[5]);
    const int16_t temp = mpu6050Be16(d[6], d[7]);
    const int16_t gx = mpu6050Be16(d[8], d[9]);
    const int16_t gy = mpu6050Be16(d[10], d[11]);
    const int16_t gz = mpu6050Be16(d[12], d[13]);

    MpuRead r{};
    r.ax_g = mpu6050AccelG(ax);
    r.ay_g = mpu6050AccelG(ay);
    r.az_g = mpu6050AccelG(az);
    r.gx_dps = mpu6050GyroDps(gx);
    r.gy_dps = mpu6050GyroDps(gy);
    r.gz_dps = mpu6050GyroDps(gz);
    r.temp_c = mpu6050TempC(temp);
    return r;
}
