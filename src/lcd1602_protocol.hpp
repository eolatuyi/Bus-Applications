#pragma once
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>

constexpr std::size_t kLcdCols = 16;

// HD44780 4-bit init nibbles (function-set high nibble 0x3, then 4-bit 0x2).
constexpr uint8_t kLcdNibble8Bit = 0x03;
constexpr uint8_t kLcdNibble4Bit = 0x02;

constexpr uint8_t kLcdCmdFunctionSet = 0x28;  // 4-bit, 2 lines, 5x8
constexpr uint8_t kLcdCmdDisplayOff = 0x08;
constexpr uint8_t kLcdCmdClear = 0x01;
constexpr uint8_t kLcdCmdEntryMode = 0x06;
constexpr uint8_t kLcdCmdDisplayOn = 0x0C;
constexpr uint8_t kLcdCmdSetDdram = 0x80;

inline uint8_t lcd1602HighNibble(uint8_t byte) {
    return static_cast<uint8_t>((byte >> 4) & 0x0F);
}

inline uint8_t lcd1602LowNibble(uint8_t byte) {
    return static_cast<uint8_t>(byte & 0x0F);
}

inline uint8_t lcd1602DdramAddr(uint8_t row, uint8_t col) {
    if (row > 1) {
        throw std::invalid_argument("LCD row out of range (0-1)");
    }
    if (col > 15) {
        throw std::invalid_argument("LCD col out of range (0-15)");
    }
    return static_cast<uint8_t>((row == 0 ? 0x00 : 0x40) + col);
}

inline uint8_t lcd1602SetDdramCommand(uint8_t row, uint8_t col) {
    return static_cast<uint8_t>(kLcdCmdSetDdram | lcd1602DdramAddr(row, col));
}

// Pad or truncate so a write from column 0 covers the visible 16x2 row.
inline std::string lcd1602FitLine(std::string s) {
    if (s.size() > kLcdCols) {
        s.resize(kLcdCols);
    } else if (s.size() < kLcdCols) {
        s.append(kLcdCols - s.size(), ' ');
    }
    return s;
}
