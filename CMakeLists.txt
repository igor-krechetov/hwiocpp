# This is an example showing how to use hsmcpp library directly from GitHub

cmake_minimum_required(VERSION 2.6)
project(example)
set(CMAKE_CXX_STANDARD 14)

# set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Toolchain-RP4.cmake)

# ----------------------------------------------------------------------
set(LIB_BINARY "hwiocpp")

add_library(${LIB_BINARY} STATIC ${CMAKE_CURRENT_SOURCE_DIR}/src/GenericDevice.cpp
                                 ${CMAKE_CURRENT_SOURCE_DIR}/src/gpio/DeviceGPIO.cpp
                                 ${CMAKE_CURRENT_SOURCE_DIR}/src/gpio/Relay.cpp
                                 ${CMAKE_CURRENT_SOURCE_DIR}/src/gpio/74hc4051.cpp
                                 ${CMAKE_CURRENT_SOURCE_DIR}/src/gpio/KeypadMatrix.cpp
                                 ${CMAKE_CURRENT_SOURCE_DIR}/src/i2c/DeviceI2C.cpp
                                 ${CMAKE_CURRENT_SOURCE_DIR}/src/i2c/sensors/aht10.cpp
                                 ${CMAKE_CURRENT_SOURCE_DIR}/src/i2c/sensors/SoilMoistureSensor.cpp
                                 ${CMAKE_CURRENT_SOURCE_DIR}/src/i2c/ads1x15.cpp)

target_compile_definitions(${LIB_BINARY} PUBLIC -DLOGGING_MODE_STRICT_VERBOSE)

target_include_directories(${LIB_BINARY}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/hwiocpp
        ${CMAKE_CURRENT_SOURCE_DIR}/modules/devutils/include
)

target_link_libraries(${LIB_BINARY} PRIVATE i2c gpiod)
