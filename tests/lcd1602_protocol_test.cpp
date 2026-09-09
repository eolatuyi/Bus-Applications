#include "lcd1602_protocol.hpp"
#include <cassert>
#include <iostream>
#include <stdexcept>

int main() {
    assert(lcd1602HighNibble(0x28) == 0x02);
    assert(lcd1602LowNibble(0x28) == 0x08);
    assert(lcd1602DdramAddr(0, 0) == 0x00);
    assert(lcd1602DdramAddr(1, 0) == 0x40);
    assert(lcd1602DdramAddr(1, 15) == 0x4F);
    assert(lcd1602SetDdramCommand(0, 3) == (0x80 | 0x03));

    assert(kLcdNibble8Bit == 0x03);
    assert(kLcdNibble4Bit == 0x02);
    assert(kLcdCmdFunctionSet == 0x28);
    assert(kLcdCmdDisplayOn == 0x0C);

    try {
        lcd1602DdramAddr(2, 0);
        std::cerr << "expected row OOB\n";
        return 1;
    } catch (const std::invalid_argument&) {
    }
    try {
        lcd1602DdramAddr(0, 16);
        std::cerr << "expected col OOB\n";
        return 1;
    } catch (const std::invalid_argument&) {
    }

    std::cout << "lcd1602_protocol_test: OK\n";
    return 0;
}
