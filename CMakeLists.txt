cmake_minimum_required(VERSION 3.10)
project(multi_bus_dashboard CXX)
set(CMAKE_CXX_STANDARD 17)

add_executable(app
    src/i2c_device.cpp src/i2c_device.hpp
    src/spi_device.cpp src/spi_device.hpp
    src/mpu6050.cpp src/mpu6050.hpp
    src/ads7830.cpp src/ads7830.hpp
    src/hc595.cpp src/hc595.hpp
    src/lcd1602.cpp src/lcd1602.hpp
    src/app.cpp
)

find_library(GPIOD_LIB gpiod)
if (NOT GPIOD_LIB)
    message(FATAL_ERROR "libgpiod not found. Install libgpiod-dev")
endif()

find_library(I2C_LIB i2c)
if (NOT I2C_LIB)
    message(FATAL_ERROR "libi2c not found. Install libi2c-dev (i2c-tools development library)")
endif()

target_link_libraries(app ${GPIOD_LIB} ${I2C_LIB})

enable_testing()

add_executable(ads7830_protocol_test tests/ads7830_protocol_test.cpp)
target_include_directories(ads7830_protocol_test PRIVATE src)
add_test(NAME ads7830_protocol_test COMMAND ads7830_protocol_test)

add_executable(hc595_bar_test tests/hc595_bar_test.cpp)
target_include_directories(hc595_bar_test PRIVATE src)
add_test(NAME hc595_bar_test COMMAND hc595_bar_test)
