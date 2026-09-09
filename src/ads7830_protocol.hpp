#pragma once
#include <cstdint>
#include <stdexcept>
#include <thread>
#include <chrono>

constexpr int kAds7830ReadRetries = 3;
constexpr auto kAds7830RetryDelay = std::chrono::milliseconds(10);

// Freenove/TI ADS7830 single-ended conversion command (base 0x84 + channel nibble).
inline uint8_t ads7830ChannelCommand(uint8_t ch) {
    if (ch > 7) {
        throw std::invalid_argument("ADS7830 channel out of range (0-7)");
    }
    return static_cast<uint8_t>(
        0x84 | ((((ch << 2) | (ch >> 1)) & 0x07) << 4));
}

inline uint8_t ads7830DecodeRawByte(int v) {
    if (v < 0) {
        throw std::runtime_error("ADS7830 read failed");
    }
    return static_cast<uint8_t>(v);
}

template <typename Attempt>
uint8_t ads7830ReadWithRetry(Attempt&& attempt, int retries,
                             std::chrono::milliseconds delay) {
    if (retries < 1) {
        throw std::invalid_argument("ADS7830 retries must be >= 1");
    }
    std::runtime_error last{"ADS7830 read failed"};
    for (int i = 0; i < retries; ++i) {
        try {
            return attempt();
        } catch (const std::exception& ex) {
            last = std::runtime_error(ex.what());
            if (i + 1 < retries) {
                std::this_thread::sleep_for(delay);
            }
        }
    }
    throw last;
}
