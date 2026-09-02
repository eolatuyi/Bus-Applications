#include "ads7830.hpp"
#include "ads7830_protocol.hpp"
#include <stdexcept>
#include <thread>
#include <chrono>

namespace {
constexpr int kReadRetries = 3;
constexpr auto kRetryDelay = std::chrono::milliseconds(10);
}  // namespace

uint8_t ADS7830::readAnalog(uint8_t ch) {
    const uint8_t cmd = ads7830ChannelCommand(ch);
    std::runtime_error last{"ADS7830 read failed"};

    for (int attempt = 0; attempt < kReadRetries; ++attempt) {
        try {
            dev_.writeRawByte(cmd);
            int v = dev_.readRawByte();
            if (v < 0) {
                throw std::runtime_error("ADS7830 read failed");
            }
            return static_cast<uint8_t>(v);
        } catch (const std::exception& ex) {
            last = std::runtime_error(ex.what());
            if (attempt + 1 < kReadRetries) {
                std::this_thread::sleep_for(kRetryDelay);
            }
        }
    }
    throw last;
}
