#pragma once
#include <cstdint>
#include <stdexcept>

// Maps 8-bit analog sample to a 74HC595 bar pattern (LEDs 0..7).
inline uint8_t hc595BarFromAnalog(uint8_t analog) {
    int leds = (static_cast<int>(analog) * 8) / 256;
    uint8_t bar = 0;
    for (int i = 0; i < leds; ++i) {
        bar |= static_cast<uint8_t>(1u << i);
    }
    return bar;
}

inline uint8_t hc595WalkBit(int index) {
    if (index < 0 || index > 7) {
        throw std::invalid_argument("HC595 walk bit out of range (0-7)");
    }
    return static_cast<uint8_t>(1u << index);
}

// n LEDs on from Q0, n in 0..8.
inline uint8_t hc595BarFill(int n) {
    if (n < 0 || n > 8) {
        throw std::invalid_argument("HC595 bar fill out of range (0-8)");
    }
    if (n == 0) {
        return 0;
    }
    return static_cast<uint8_t>((1u << n) - 1);
}
