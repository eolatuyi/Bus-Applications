#include "hc595.hpp"

void HC595::writeBarFromAnalog(uint8_t a0) {
    int leds = (a0 * 8) / 256;
    uint8_t bar = 0;
    for (int i = 0; i < leds; ++i) bar |= (1 << i);
    writeByte(bar);
}
