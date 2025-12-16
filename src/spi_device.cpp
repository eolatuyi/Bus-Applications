#include "spi_device.hpp"
#include <stdexcept>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

SPIDevice::SPIDevice(const char* dev, uint32_t speed) {
    fd_ = open(dev, O_RDWR);
    if (fd_ < 0) throw std::runtime_error("Open SPI failed");
    uint8_t mode = SPI_MODE_0;
    uint8_t bits = 8;
    if (ioctl(fd_, SPI_IOC_WR_MODE, &mode) < 0) throw std::runtime_error("Set SPI mode failed");
    if (ioctl(fd_, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0) throw std::runtime_error("Set bits failed");
    if (ioctl(fd_, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) throw std::runtime_error("Set speed failed");
}
SPIDevice::~SPIDevice() { if (fd_ >= 0) close(fd_); }

void SPIDevice::transfer(const std::vector<uint8_t>& tx) {
    struct spi_ioc_transfer tr{};
    tr.tx_buf = (unsigned long)tx.data();
    tr.rx_buf = 0;
    tr.len = tx.size();
    tr.cs_change = 0;
    if (ioctl(fd_, SPI_IOC_MESSAGE(1), &tr) < 0) throw std::runtime_error("SPI transfer failed");
}
