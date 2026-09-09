#pragma once
#include <cstdint>
#include <stdexcept>

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

// Maps 8-bit analog sample to a 74HC595 bar pattern (LEDs 0..8).
// Divide by 255 so analog 255 lights all eight segments (Q0–Q7).
inline uint8_t hc595BarFromAnalog(uint8_t analog) {
    const int leds = (static_cast<int>(analog) * 8) / 255;
    return hc595BarFill(leds);
}
