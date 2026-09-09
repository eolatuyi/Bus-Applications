#include "ads7830.hpp"
#include "ads7830_protocol.hpp"

uint8_t ADS7830::readAnalog(uint8_t ch) {
    const uint8_t cmd = ads7830ChannelCommand(ch);
    return ads7830ReadWithRetry(
        [this, cmd]() {
            dev_.writeRawByte(cmd);
            return ads7830DecodeRawByte(dev_.readRawByte());
        },
        kAds7830ReadRetries, kAds7830RetryDelay);
}
