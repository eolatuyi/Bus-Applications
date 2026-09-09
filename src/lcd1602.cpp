#include "lcd1602.hpp"
#include "lcd1602_protocol.hpp"
#include <stdexcept>
#include <string>
#include <gpiod.h>
#include <thread>
#include <chrono>

LCD1602::LCD1602(int rs, int e, int d4, int d5, int d6, int d7, const char* chip_path)
    : chip_(nullptr), request_(nullptr),
      rs_off_(static_cast<unsigned int>(rs)),
      e_off_(static_cast<unsigned int>(e)),
      d4_off_(static_cast<unsigned int>(d4)),
      d5_off_(static_cast<unsigned int>(d5)),
      d6_off_(static_cast<unsigned int>(d6)),
      d7_off_(static_cast<unsigned int>(d7)) {
    chip_ = gpiod_chip_open(chip_path);
    if (!chip_) throw std::runtime_error(std::string("Failed to open GPIO chip: ") + chip_path);

    struct gpiod_request_config* req_cfg = nullptr;
    struct gpiod_line_config* line_cfg = nullptr;
    struct gpiod_line_settings* line_settings = nullptr;

    auto cleanup_cfg = [&]() {
        if (line_settings) {
            gpiod_line_settings_free(line_settings);
            line_settings = nullptr;
        }
        if (line_cfg) {
            gpiod_line_config_free(line_cfg);
            line_cfg = nullptr;
        }
        if (req_cfg) {
            gpiod_request_config_free(req_cfg);
            req_cfg = nullptr;
        }
    };

    auto fail = [&](const std::string& msg) {
        cleanup_cfg();
        gpiod_chip_close(chip_);
        chip_ = nullptr;
        throw std::runtime_error(msg);
    };

    req_cfg = gpiod_request_config_new();
    if (!req_cfg) fail("Failed to create request config");
    gpiod_request_config_set_consumer(req_cfg, "LCD1602");

    line_cfg = gpiod_line_config_new();
    if (!line_cfg) fail("Failed to create line config");

    line_settings = gpiod_line_settings_new();
    if (!line_settings) fail("Failed to create line settings");
    gpiod_line_settings_set_direction(line_settings, GPIOD_LINE_DIRECTION_OUTPUT);
    gpiod_line_settings_set_output_value(line_settings, GPIOD_LINE_VALUE_INACTIVE);

    unsigned int offsets[6] = {rs_off_, e_off_, d4_off_, d5_off_, d6_off_, d7_off_};
    if (gpiod_line_config_add_line_settings(line_cfg, offsets, 6, line_settings) < 0) {
        fail("Failed to add line settings");
    }
    gpiod_line_settings_free(line_settings);
    line_settings = nullptr;

    request_ = gpiod_chip_request_lines(chip_, req_cfg, line_cfg);
    cleanup_cfg();
    if (!request_) {
        gpiod_chip_close(chip_);
        chip_ = nullptr;
        throw std::runtime_error(std::string("Request LCD GPIO lines failed: ") + chip_path);
    }
}

LCD1602::~LCD1602() {
    if (request_) gpiod_line_request_release(request_);
    if (chip_) gpiod_chip_close(chip_);
}

void LCD1602::delay_us(unsigned int us) {
    std::this_thread::sleep_for(std::chrono::microseconds(us));
}

void LCD1602::setLine(unsigned int offset, bool on) {
    // libgpiod v2: offset is the chip line number used when requesting, not 0.
    int rc = gpiod_line_request_set_value(
        request_, offset, on ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE);
    if (rc < 0) {
        throw std::runtime_error("GPIO set_value failed for offset " + std::to_string(offset));
    }
}

void LCD1602::pulseEnable() {
    // HD44780 needs >= ~450 ns; use generous pulse for 5V modules / long jumpers.
    setLine(e_off_, false);
    delay_us(1);
    setLine(e_off_, true);
    delay_us(10);
    setLine(e_off_, false);
    delay_us(100);
}

void LCD1602::write4(uint8_t nibble, bool rs) {
    setLine(rs_off_, rs);
    setLine(d4_off_, (nibble >> 0) & 0x1);
    setLine(d5_off_, (nibble >> 1) & 0x1);
    setLine(d6_off_, (nibble >> 2) & 0x1);
    setLine(d7_off_, (nibble >> 3) & 0x1);
    pulseEnable();
}

void LCD1602::write8(uint8_t byte, bool rs) {
    write4(lcd1602HighNibble(byte), rs);
    write4(lcd1602LowNibble(byte), rs);
}

void LCD1602::init() {
    // HD44780 power-on + 4-bit init (datasheet + extra margin for cold 5V modules).
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    write4(kLcdNibble8Bit, false);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    write4(kLcdNibble8Bit, false);
    delay_us(200);
    write4(kLcdNibble8Bit, false);
    delay_us(200);
    write4(kLcdNibble4Bit, false);  // switch to 4-bit
    delay_us(200);
    cmd(kLcdCmdFunctionSet);
    cmd(kLcdCmdDisplayOff);
    cmd(kLcdCmdClear);
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    cmd(kLcdCmdEntryMode);
    cmd(kLcdCmdDisplayOn);
}

void LCD1602::clear() {
    cmd(kLcdCmdClear);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
}

void LCD1602::setCursor(uint8_t row, uint8_t col) {
    cmd(lcd1602SetDdramCommand(row, col));
}

void LCD1602::print(const std::string& s) {
    for (char c : s) data(static_cast<uint8_t>(c));
}
