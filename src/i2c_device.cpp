#include "i2c_device.hpp"
#include <stdexcept>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>

I2CDevice::I2CDevice(int bus, uint8_t addr) : fd_(-1), bus_(bus), addr_(addr) { openBus(); }
I2CDevice::~I2CDevice() { if (fd_ >= 0) close(fd_); }

void I2CDevice::openBus() {
    std::string dev = "/dev/i2c-" + std::to_string(bus_);
    fd_ = open(dev.c_str(), O_RDWR);
    if (fd_ < 0) throw std::runtime_error("Failed to open " + dev);
    if (ioctl(fd_, I2C_SLAVE, addr_) < 0) throw std::runtime_error("Failed to set I2C addr");
}
void I2CDevice::setAddress(uint8_t addr) {
    addr_ = addr;
    if (ioctl(fd_, I2C_SLAVE, addr_) < 0) throw std::runtime_error("Failed to set I2C addr");
}

void I2CDevice::writeByte(uint8_t reg, uint8_t val) {
    uint8_t buf[2] = {reg, val};
    if (write(fd_, buf, 2) != 2) throw std::runtime_error("I2C writeByte failed");
}
void I2CDevice::writeBytes(uint8_t reg, const std::vector<uint8_t>& data) {
    std::vector<uint8_t> buf; buf.reserve(1 + data.size());
    buf.push_back(reg);
    buf.insert(buf.end(), data.begin(), data.end());
    if (write(fd_, buf.data(), buf.size()) != (ssize_t)buf.size())
        throw std::runtime_error("I2C writeBytes failed");
}
uint8_t I2CDevice::readByte(uint8_t reg) {
    if (write(fd_, &reg, 1) != 1) throw std::runtime_error("I2C set reg failed");
    uint8_t val;
    if (read(fd_, &val, 1) != 1) throw std::runtime_error("I2C readByte failed");
    return val;
}
std::vector<uint8_t> I2CDevice::readBytes(uint8_t reg, size_t len) {
    if (write(fd_, &reg, 1) != 1) throw std::runtime_error("I2C set reg failed");
    std::vector<uint8_t> buf(len);
    if (read(fd_, buf.data(), len) != (ssize_t)len) throw std::runtime_error("I2C readBytes failed");
    return buf;
}

void I2CDevice::writeRawByte(uint8_t b) {
    if (i2c_smbus_write_byte(fd_, b) < 0) throw std::runtime_error("I2C writeRawByte failed");
}
int I2CDevice::readRawByte() {
    int v = i2c_smbus_read_byte(fd_);
    return v;
}
