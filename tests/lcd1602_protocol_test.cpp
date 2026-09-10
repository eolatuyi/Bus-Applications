#include "lcd1602_protocol.hpp"
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <string>

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

    assert(kLcdCols == 16);
    assert(lcd1602FitLine("") == std::string(16, ' '));
    assert(lcd1602FitLine("GPIO lines live") == "GPIO lines live ");
    assert(lcd1602FitLine("LCD bring-up OK ") == "LCD bring-up OK ");
    assert(lcd1602FitLine("I2C/SPI Dashboard") == "I2C/SPI Dashboar");
    assert(lcd1602FitLine("Pot:  0") == "Pot:  0         ");
    assert(lcd1602FitLine("I2C/SPI Dashboard").size() == 16);
    assert(lcd1602FitLine("Pot:  0").back() == ' ');

    std::cout << "lcd1602_protocol_test: OK\n";
    return 0;
}
