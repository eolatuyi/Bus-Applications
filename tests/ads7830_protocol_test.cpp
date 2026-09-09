#include "ads7830_protocol.hpp"
#include <cassert>
#include <cstdint>
#include <iostream>
#include <stdexcept>

int main() {
    try {
        ads7830ChannelCommand(8);
        std::cerr << "expected invalid_argument for ch=8\n";
        return 1;
    } catch (const std::invalid_argument&) {
    }

    assert(ads7830ChannelCommand(0) == 0x84);
    assert(ads7830ChannelCommand(1) == 0xC4);
    assert(ads7830ChannelCommand(2) == 0x94);  // Freenove pot channel
    assert(ads7830ChannelCommand(7) == 0xF4);

    assert(ads7830DecodeRawByte(0) == 0);
    assert(ads7830DecodeRawByte(255) == 255);
    try {
        ads7830DecodeRawByte(-1);
        std::cerr << "expected runtime_error for negative raw byte\n";
        return 1;
    } catch (const std::runtime_error&) {
    }

    int calls = 0;
    uint8_t v = ads7830ReadWithRetry(
        [&calls]() {
            ++calls;
            if (calls < 3) {
                throw std::runtime_error("transient");
            }
            return static_cast<uint8_t>(42);
        },
        kAds7830ReadRetries, std::chrono::milliseconds(0));
    assert(v == 42);
    assert(calls == 3);

    calls = 0;
    try {
        ads7830ReadWithRetry(
            [&calls]() -> uint8_t {
                ++calls;
                throw std::runtime_error("always");
            },
            kAds7830ReadRetries, std::chrono::milliseconds(0));
        std::cerr << "expected throw after retries exhausted\n";
        return 1;
    } catch (const std::runtime_error&) {
        assert(calls == kAds7830ReadRetries);
    }

    std::cout << "ads7830_protocol_test: OK\n";
    return 0;
}
