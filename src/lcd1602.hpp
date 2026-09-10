#pragma once
#include <string>
#include <cstdint>

class LCD1602 {
public:
    LCD1602(int rs=17, int e=27, int d4=22, int d5=23, int d6=24, int d7=25,
            const char* chip_path="/dev/gpiochip0");
    ~LCD1602();

    LCD1602(const LCD1602&) = delete;
    LCD1602& operator=(const LCD1602&) = delete;
    LCD1602(LCD1602&&) = delete;
    LCD1602& operator=(LCD1602&&) = delete;

    void init();
    void clear();
    void setCursor(uint8_t row, uint8_t col);
    void print(const std::string& s);
    void printLine(uint8_t row, const std::string& s);

private:
    struct gpiod_chip* chip_;
    struct gpiod_line_request* request_;
    unsigned int rs_off_, e_off_, d4_off_, d5_off_, d6_off_, d7_off_;

    void pulseEnable();
    void write4(uint8_t nibble, bool rs);
    void write8(uint8_t byte, bool rs);
    void cmd(uint8_t c) { write8(c, false); }
    void data(uint8_t d) { write8(d, true); }
    void delay_us(unsigned int us);
    void setLine(unsigned int offset, bool on);
};
