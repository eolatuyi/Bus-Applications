#pragma once
#include <cstdint>

// Maps 8-bit analog sample to a 74HC595 bar pattern (LEDs 0..7).
inline uint8_t hc595BarFromAnalog(uint8_t analog) {
    int leds = (static_cast<int>(analog) * 8) / 256;
    uint8_t bar = 0;
    for (int i = 0; i < leds; ++i) {
        bar |= static_cast<uint8_t>(1u << i);
    }
    return bar;
}
