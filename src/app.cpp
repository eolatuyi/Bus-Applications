#include <iostream>
#include <thread>
#include <chrono>
#include "mpu6050.hpp"
#include "pcf8591.hpp"
#include "hc595.hpp"
#include "lcd1602.hpp"

int main() {
    try {
        MPU6050 imu(0x68);
        imu.init();
        PCF8591 adc(0x48);
        HC595 sr;
        LCD1602 lcd;
        lcd.init();
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("I2C/SPI Dashboard");

        while (true) {
            auto r = imu.read();
            uint8_t a0 = adc.readAnalog(0);
            sr.writeBarFromAnalog(a0);

            std::cout << "Accel[g]=" << r.ax_g << "," << r.ay_g << "," << r.az_g
                      << " Gyro[dps]=" << r.gz_dps
                      << " Temp[C]=" << r.temp_c
                      << " A0=" << (int)a0 << std::endl;

            lcd.setCursor(0,0);
            char buf1[17];
            snprintf(buf1, sizeof(buf1), "A0:%3d        ", (int)a0);
            lcd.print(buf1);
            lcd.setCursor(1,0);
            char buf2[17];
            snprintf(buf2, sizeof(buf2), "ax:%+1.2fg     ", r.ax_g);
            lcd.print(buf2);

            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }
}
