#include "hc595_bar.hpp"
#include <cassert>
#include <cstdint>
#include <iostream>

int main() {
    assert(hc595BarFromAnalog(0) == 0x00);
    assert(hc595BarFromAnalog(31) == 0x00);
    assert(hc595BarFromAnalog(32) == 0x01);
    assert(hc595BarFromAnalog(255) == 0x7F);  // (255*8)/256 → 7 LEDs
    assert(hc595BarFromAnalog(128) == 0x0F);

    std::cout << "hc595_bar_test: OK\n";
    return 0;
}
