#include "hc595_bar.hpp"
#include <cassert>
#include <cstdint>
#include <iostream>
#include <stdexcept>

int main() {
    assert(hc595BarFromAnalog(0) == 0x00);
    assert(hc595BarFromAnalog(31) == 0x00);
    assert(hc595BarFromAnalog(32) == 0x01);
    assert(hc595BarFromAnalog(255) == 0x7F);  // (255*8)/256 → 7 LEDs
    assert(hc595BarFromAnalog(128) == 0x0F);

    assert(hc595WalkBit(0) == 0x01);
    assert(hc595WalkBit(7) == 0x80);
    assert(hc595BarFill(0) == 0x00);
    assert(hc595BarFill(1) == 0x01);
    assert(hc595BarFill(8) == 0xFF);
    try {
        hc595WalkBit(8);
        return 1;
    } catch (const std::invalid_argument&) {
    }

    std::cout << "hc595_bar_test: OK\n";
    return 0;
}
