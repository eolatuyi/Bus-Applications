#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>
#include <memory>
#include <algorithm>
#include <iomanip>
#include "mpu6050.hpp"
#include "ads7830.hpp"
#include "hc595.hpp"
#include "lcd1602.hpp"

namespace {
constexpr auto kPollInterval = std::chrono::milliseconds(200);
constexpr auto kSensorRetryDelay = std::chrono::milliseconds(50);
constexpr int kStalePotSamples = 25;  // 5 s at 200 ms — warn if pot never moves

void printUsage(const char* prog) {
    std::cerr << "Usage: " << prog << " [--no-lcd]\n";
}

void printHardwareBanner() {
    std::cout << "ADS7830 @" << std::hex << std::showbase
              << static_cast<int>(ADS7830::kAddr)
              << " pot=CH" << std::dec
              << static_cast<int>(ADS7830::kPotChannel)
              << " (Freenove projects board)\n";
}
}  // namespace

int main(int argc, char* argv[]) {
    bool use_lcd = true;

    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--no-lcd") == 0) {
            use_lcd = false;
        } else {
            printUsage(argv[0]);
            return 1;
        }
    }

    try {
        MPU6050 imu(0x68);
        imu.init();
        ADS7830 adc;
        HC595 sr;
        std::unique_ptr<LCD1602> lcd;
        if (use_lcd) {
            lcd = std::make_unique<LCD1602>();
            lcd->init();
            lcd->clear();
            lcd->setCursor(0, 0);
            lcd->print("I2C/SPI Dashboard");
        } else {
            std::cout << "LCD disabled (--no-lcd)\n";
        }

        printHardwareBanner();

        int potSamples = 0;
        uint8_t potMin = 255;
        uint8_t potMax = 0;
        bool stalePotWarned = false;

        while (true) {
            try {
                auto r = imu.read();
                uint8_t pot = adc.readAnalog(ADS7830::kPotChannel);
                sr.writeBarFromAnalog(pot);

                potMin = std::min(potMin, pot);
                potMax = std::max(potMax, pot);
                ++potSamples;
                if (!stalePotWarned && potSamples >= kStalePotSamples &&
                    potMin == potMax) {
                    std::cerr << "Warning: pot reading unchanged on ADS7830 CH"
                              << static_cast<int>(ADS7830::kPotChannel)
                              << " — turn the knob or verify Freenove CH2 wiring\n";
                    stalePotWarned = true;
                }

                std::cout << "Accel[g]=" << r.ax_g << "," << r.ay_g << "," << r.az_g
                          << " Gyro[dps]=" << r.gz_dps
                          << " Temp[C]=" << r.temp_c
                          << " Pot=" << static_cast<int>(pot) << std::endl;

                if (lcd) {
                    lcd->setCursor(0, 0);
                    char buf1[17];
                    snprintf(buf1, sizeof(buf1), "Pot:%3d        ",
                             static_cast<int>(pot));
                    lcd->print(buf1);
                    lcd->setCursor(1, 0);
                    char buf2[17];
                    snprintf(buf2, sizeof(buf2), "ax:%+1.2fg     ", r.ax_g);
                    lcd->print(buf2);
                }
            } catch (const std::exception& ex) {
                std::cerr << "Sensor error (retrying): " << ex.what() << std::endl;
                std::this_thread::sleep_for(kSensorRetryDelay);
                continue;
            }

            std::this_thread::sleep_for(kPollInterval);
        }
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }
}
