#pragma once
#include <string>
#include <vector>
#include <cstdint>

class LCD1602 {
public:
    LCD1602(int rs=17, int e=27, int d4=22, int d5=23, int d6=24, int d7=25,
            const char* chip_name="gpiochip0");
    ~LCD1602();

    void init();
    void clear();
    void setCursor(uint8_t row, uint8_t col);
    void print(const std::string& s);

private:
    struct gpiod_chip* chip_;
    struct gpiod_line* rs_;
    struct gpiod_line* e_;
    struct gpiod_line* d4_;
    struct gpiod_line* d5_;
    struct gpiod_line* d6_;
    struct gpiod_line* d7_;

    void pulseEnable();
    void write4(uint8_t nibble, bool rs);
    void write8(uint8_t byte, bool rs);
    void cmd(uint8_t c) { write8(c, false); }
    void data(uint8_t d) { write8(d, true); }
    void delay_us(unsigned int us);
};
