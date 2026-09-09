#include "lcd1602.hpp"
#include <stdexcept>
#include <string>
#include <gpiod.h>
#include <thread>
#include <chrono>

LCD1602::LCD1602(int rs, int e, int d4, int d5, int d6, int d7, const char* chip_path)
    : chip_(nullptr), rs_(nullptr), e_(nullptr), d4_(nullptr), d5_(nullptr), d6_(nullptr), d7_(nullptr),
      rs_off_(static_cast<unsigned int>(rs)),
      e_off_(static_cast<unsigned int>(e)),
      d4_off_(static_cast<unsigned int>(d4)),
      d5_off_(static_cast<unsigned int>(d5)),
      d6_off_(static_cast<unsigned int>(d6)),
      d7_off_(static_cast<unsigned int>(d7)) {
    chip_ = gpiod_chip_open(chip_path);
    if (!chip_) throw std::runtime_error(std::string("Failed to open GPIO chip: ") + chip_path);

    auto request_out = [&](unsigned int gpio, const char* name) {
        struct gpiod_request_config* req_cfg = gpiod_request_config_new();
        if (!req_cfg) throw std::runtime_error("Failed to create request config");

        gpiod_request_config_set_consumer(req_cfg, name);

        struct gpiod_line_config* line_cfg = gpiod_line_config_new();
        if (!line_cfg) {
            gpiod_request_config_free(req_cfg);
            throw std::runtime_error("Failed to create line config");
        }

        unsigned int offsets[1] = {gpio};
        struct gpiod_line_settings* line_settings = gpiod_line_settings_new();
        if (!line_settings) {
            gpiod_line_config_free(line_cfg);
            gpiod_request_config_free(req_cfg);
            throw std::runtime_error("Failed to create line settings");
        }

        gpiod_line_settings_set_direction(line_settings, GPIOD_LINE_DIRECTION_OUTPUT);
        gpiod_line_settings_set_output_value(line_settings, GPIOD_LINE_VALUE_INACTIVE);

        int rc = gpiod_line_config_add_line_settings(line_cfg, offsets, 1, line_settings);
        gpiod_line_settings_free(line_settings);
        if (rc < 0) {
            gpiod_line_config_free(line_cfg);
            gpiod_request_config_free(req_cfg);
            throw std::runtime_error("Failed to add line settings");
        }

        struct gpiod_line_request* request = gpiod_chip_request_lines(chip_, req_cfg, line_cfg);
        gpiod_request_config_free(req_cfg);
        gpiod_line_config_free(line_cfg);

        if (!request) throw std::runtime_error(std::string("Request output failed: ") + name);
        return request;
    };

    rs_ = request_out(rs_off_, "LCD_RS");
    e_  = request_out(e_off_,  "LCD_E");
    d4_ = request_out(d4_off_, "LCD_D4");
    d5_ = request_out(d5_off_, "LCD_D5");
    d6_ = request_out(d6_off_, "LCD_D6");
    d7_ = request_out(d7_off_, "LCD_D7");
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

void LCD1602::setLine(struct gpiod_line_request* req, unsigned int offset, bool on) {
    // libgpiod v2: offset is the chip line number used when requesting, not 0.
    int rc = gpiod_line_request_set_value(
        req, offset, on ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE);
    if (rc < 0) {
        throw std::runtime_error("GPIO set_value failed for offset " + std::to_string(offset));
    }
}

void LCD1602::pulseEnable() {
    // HD44780 needs >= ~450 ns; use generous pulse for 5V modules / long jumpers.
    setLine(e_, e_off_, false);
    delay_us(1);
    setLine(e_, e_off_, true);
    delay_us(10);
    setLine(e_, e_off_, false);
    delay_us(100);
}

void LCD1602::write4(uint8_t nibble, bool rs) {
    setLine(rs_, rs_off_, rs);
    setLine(d4_, d4_off_, (nibble >> 0) & 0x1);
    setLine(d5_, d5_off_, (nibble >> 1) & 0x1);
    setLine(d6_, d6_off_, (nibble >> 2) & 0x1);
    setLine(d7_, d7_off_, (nibble >> 3) & 0x1);
    pulseEnable();
}

void LCD1602::write8(uint8_t byte, bool rs) {
    write4((byte >> 4) & 0x0F, rs);
    write4(byte & 0x0F, rs);
}

void LCD1602::init() {
    // HD44780 power-on + 4-bit init (datasheet + extra margin for cold 5V modules).
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    write4(0x03, false);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    write4(0x03, false);
    delay_us(200);
    write4(0x03, false);
    delay_us(200);
    write4(0x02, false);  // switch to 4-bit
    delay_us(200);
    cmd(0x28);  // 4-bit, 2 lines, 5x8
    cmd(0x08);  // display off
    cmd(0x01);  // clear
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    cmd(0x06);  // entry mode
    cmd(0x0C);  // display on, cursor off
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
