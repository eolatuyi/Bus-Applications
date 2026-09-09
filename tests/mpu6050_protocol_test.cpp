#include "mpu6050_protocol.hpp"
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>

int main() {
    assert(mpu6050WhoAmIOk(0x68));
    assert(!mpu6050WhoAmIOk(0x00));
    assert(!mpu6050WhoAmIOk(0x70));

    assert(mpu6050Be16(0x40, 0x00) == 16384);
    assert(std::fabs(mpu6050AccelG(16384) - 1.0f) < 1e-6f);
    assert(std::fabs(mpu6050GyroDps(131) - 1.0f) < 1e-5f);
    assert(std::fabs(mpu6050TempC(0) - 36.53f) < 1e-4f);

    std::vector<uint8_t> rest(14, 0);
    rest[4] = 0x40;  // az = 16384 → ~1 g
    rest[5] = 0x00;
    MpuRead r = mpu6050Decode(rest);
    assert(std::fabs(r.az_g - 1.0f) < 1e-6f);
    assert(std::fabs(r.ax_g) < 1e-6f);

    std::vector<uint8_t> stuck(14, 0xFF);
    assert(mpu6050BurstLooksStuck(stuck));
    try {
        mpu6050Decode(stuck);
        std::cerr << "expected stuck-burst error\n";
        return 1;
    } catch (const std::runtime_error&) {
    }

    try {
        mpu6050Decode(std::vector<uint8_t>(13, 0));
        std::cerr << "expected short-burst error\n";
        return 1;
    } catch (const std::invalid_argument&) {
    }

    std::cout << "mpu6050_protocol_test: OK\n";
    return 0;
}
