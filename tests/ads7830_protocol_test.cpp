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

    std::cout << "ads7830_protocol_test: OK\n";
    return 0;
}
