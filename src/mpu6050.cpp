#include "mpu6050.hpp"

#include <cstdio>
#include <stdexcept>
#include <string>

MPU6050::MPU6050(uint8_t addr) : dev_(1, addr) {}

void MPU6050::init() {
    dev_.writeByte(kMpuRegPwrMgmt1, 0x00);
    const uint8_t id = dev_.readByte(kMpuRegWhoAmI);
    if (!mpu6050WhoAmIOk(id)) {
        char buf[8];
        std::snprintf(buf, sizeof(buf), "%02x", id);
        throw std::runtime_error(std::string("MPU6050 WHO_AM_I=0x") + buf +
                                 " expected 0x68");
    }
}

MpuRead MPU6050::read() {
    return mpu6050Decode(dev_.readBytes(kMpuRegAccelXoutH, 14));
}
