cmake_minimum_required(VERSION 3.10)
project(multi_bus_dashboard CXX)
set(CMAKE_CXX_STANDARD 17)

add_executable(app
    src/i2c_device.cpp src/i2c_device.hpp
    src/spi_device.cpp src/spi_device.hpp
    src/mpu6050.cpp src/mpu6050.hpp
    src/pcf8591.cpp src/pcf8591.hpp
    src/hc595.cpp src/hc595.hpp
    src/lcd1602.cpp src/lcd1602.hpp
    src/app.cpp
)

find_library(GPIOD_LIB gpiod)
if (NOT GPIOD_LIB)
    message(FATAL_ERROR "libgpiod not found. Install libgpiod-dev")
endif()

target_link_libraries(app ${GPIOD_LIB})
