#include "lcd1602.hpp"
#include <stdexcept>
#include <gpiod.h>
#include <thread>
#include <chrono>

LCD1602::LCD1602(int rs, int e, int d4, int d5, int d6, int d7, const char* chip_name)
    : chip_(nullptr), rs_(nullptr), e_(nullptr), d4_(nullptr), d5_(nullptr), d6_(nullptr), d7_(nullptr) {
    chip_ = gpiod_chip_open(chip_name);
    if (!chip_) throw std::runtime_error("Failed to open GPIO chip");

    auto request_out = [&](int gpio, const char* name){
        struct gpiod_request_config* req_cfg = gpiod_request_config_new();
        if (!req_cfg) throw std::runtime_error("Failed to create request config");
        
        gpiod_request_config_set_consumer(req_cfg, name);
        gpiod_request_config_add_line_by_offset(req_cfg, gpio);
        
        struct gpiod_line_request* request = gpiod_chip_request_lines(chip_, req_cfg);
        gpiod_request_config_free(req_cfg);
        
        if (!request) throw std::runtime_error(std::string("Request output failed: ") + name);
        return request;
    };

    rs_ = request_out(rs, "LCD_RS");
    e_  = request_out(e,  "LCD_E");
    d4_ = request_out(d4, "LCD_D4");
    d5_ = request_out(d5, "LCD_D5");
    d6_ = request_out(d6, "LCD_D6");
    d7_ = request_out(d7, "LCD_D7");
}

LCD1602::~LCD1602() {
    if (rs_) gpiod_line_request_release(rs_);
    if (e_) gpiod_line_request_release(e_);
    if (d4_) gpiod_line_request_release(d4_);
    if (d5_) gpiod_line_request_release(d5_);
    if (d6_) gpiod_line_request_release(d6_);
    if (d7_) gpiod_line_request_release(d7_);
    if (chip_) gpiod_chip_close(chip_);
}

void LCD1602::delay_us(unsigned int us) {
    std::this_thread::sleep_for(std::chrono::microseconds(us));
}

void LCD1602::pulseEnable() {
    gpiod_line_request_set_value(e_, 0, GPIOD_LINE_VALUE_ACTIVE);
    delay_us(2);
    gpiod_line_request_set_value(e_, 0, GPIOD_LINE_VALUE_INACTIVE);
    delay_us(50);
}

void LCD1602::write4(uint8_t nibble, bool rs) {
    gpiod_line_request_set_value(rs_, 0, rs ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE);
    gpiod_line_request_set_value(d4_, 0, ((nibble >> 0) & 0x1) ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE);
    gpiod_line_request_set_value(d5_, 0, ((nibble >> 1) & 0x1) ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE);
    gpiod_line_request_set_value(d6_, 0, ((nibble >> 2) & 0x1) ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE);
    gpiod_line_request_set_value(d7_, 0, ((nibble >> 3) & 0x1) ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE);
    pulseEnable();
}

void LCD1602::write8(uint8_t byte, bool rs) {
    write4((byte >> 4) & 0x0F, rs);
    write4(byte & 0x0F, rs);
}

void LCD1602::init() {
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    write4(0x03, false); std::this_thread::sleep_for(std::chrono::milliseconds(5));
    write4(0x03, false); std::this_thread::sleep_for(std::chrono::microseconds(150));
    write4(0x03, false); delay_us(150);
    write4(0x02, false);
    cmd(0x28);
    cmd(0x0C);
    cmd(0x01); std::this_thread::sleep_for(std::chrono::milliseconds(2));
    cmd(0x06);
}

void LCD1602::clear() {
    cmd(0x01);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
}

void LCD1602::setCursor(uint8_t row, uint8_t col) {
    if (row > 1) row = 1;
    if (col > 15) col = 15;
    uint8_t addr = (row == 0 ? 0x00 : 0x40) + col;
    cmd(0x80 | addr);
}

void LCD1602::print(const std::string& s) {
    for (char c : s) data(static_cast<uint8_t>(c));
}
