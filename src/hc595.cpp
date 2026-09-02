#include "hc595.hpp"
#include "hc595_bar.hpp"

void HC595::writeBarFromAnalog(uint8_t a0) {
    writeByte(hc595BarFromAnalog(a0));
}
