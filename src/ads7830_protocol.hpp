#pragma once
#include <cstdint>
#include <stdexcept>

// Freenove/TI ADS7830 single-ended conversion command (base 0x84 + channel nibble).
inline uint8_t ads7830ChannelCommand(uint8_t ch) {
    if (ch > 7) {
        throw std::invalid_argument("ADS7830 channel out of range (0-7)");
    }
    return static_cast<uint8_t>(
        0x84 | ((((ch << 2) | (ch >> 1)) & 0x07) << 4));
}
