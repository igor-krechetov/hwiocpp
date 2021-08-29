# This is an example showing how to use hsmcpp library directly from GitHub

cmake_minimum_required(VERSION 2.6)
project(example)

# export CMAKE_TOOLCHAIN_FILE=../cmake/Toolchain-RP4.cmake
# export Qt5_DIR=/home/ikrechetov/projects/rp4_qt/qt5.15/lib/cmake/Qt5
# set(CMAKE_TOOLCHAIN_FILE ./cmake/Toolchain-RP4.cmake)
list(APPEND CMAKE_FIND_ROOT_PATH "/home/ikrechetov/projects/rp4_qt/qt5.15")


set(BINARY_NAME "example")
set(CMAKE_CXX_STANDARD 14)

# wrapper function to download hsmcpp
# function(installHsmcpp)
#     include(FetchContent)
#     message("Downloading hsmcpp...")
#     FetchContent_Declare(hsmcpp
#                          GIT_REPOSITORY https://github.com/igor-krechetov/hsmcpp.git
#                          GIT_TAG main
#     )
#     FetchContent_MakeAvailable(hsmcpp)
# endfunction()


# Configure and install HSMCPP library
SET(HSMBUILD_VERBOSE OFF CACHE BOOL "Enable traces")
SET(HSMBUILD_EXAMPLES OFF CACHE BOOL "Disable HSM examples")
SET(HSMBUILD_TESTS OFF CACHE BOOL "Disable HSM tests")
SET(HSMBUILD_DISPATCHER_GLIB OFF CACHE BOOL "Disable Glib dispatcher")
SET(HSMBUILD_DISPATCHER_GLIBMM OFF CACHE BOOL "Disable Glibmm dispatcher")
SET(HSMBUILD_DISPATCHER_QT OFF CACHE BOOL "Disable Qt dispatcher")
SET(HSMBUILD_DISPATCHER_STD ON CACHE BOOL "Enable STD dispatcher")

# installHsmcpp()
add_subdirectory(./3rdparty/hsmcpp)

# ----------------------------------------------------------------------
# Application specific code. This is just an example it there are no restrictions to use it specifically in this way

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${HSMCPP_CXX_FLAGS}")
set(BINARY_NAME "myapplication")

# generate state machine code (optional)
set(GEN_DIR ${CMAKE_BINARY_DIR}/gen)
file(MAKE_DIRECTORY ${GEN_DIR})
generateHsm(GEN_HSM ./main.scxml "SwitchHsm" ${GEN_DIR} "GEN_SRC")

# add defined needed by HSMCPP (based on your dispatcher type)
add_definitions(${HSM_DEFINITIONS_STD})
add_executable(${BINARY_NAME} ${GEN_SRC}
                              ./src/main.cpp
                              ./src/devices/GenericDevice.cpp
                              ./src/devices/gpio/DeviceGPIO.cpp
                              ./src/devices/gpio/Relay.cpp
                              ./src/devices/gpio/74hc4051.cpp
                              ./src/devices/gpio/KeypadMatrix.cpp
                              ./src/devices/i2c/DeviceI2C.cpp
                              ./src/devices/i2c/sensors/aht10.cpp
                              ./src/devices/i2c/sensors/SoilMoistureSensor.cpp
                              ./src/devices/i2c/ads1x15.cpp)

add_dependencies(${BINARY_NAME} GEN_HSM)

# generate state machine diagram (optional)
set(GEN_DIR_DIAGRAM ${CMAKE_BINARY_DIR}/gen_diagram)
file(MAKE_DIRECTORY ${GEN_DIR_DIAGRAM})
generateHsmDiagram(GEN_HSM_DIAGRAM ./main.scxml ${GEN_DIR_DIAGRAM}/main.plantuml)
add_dependencies(${BINARY_NAME} GEN_HSM_DIAGRAM)
target_compile_definitions(${BINARY_NAME} PUBLIC -DLOGGING_MODE_STRICT_VERBOSE)

# target_link_directories(${BINARY_NAME} ./thirdparty/lib)
# find_library(LIB_LGPIO
#     NAMES lgpio
#     HINTS "./thirdparty/lib"
# )

target_include_directories(${BINARY_NAME}
    PRIVATE
        ${HSMCPP_STD_INCLUDE} # add HSMCPP to include path
        ${CMAKE_BINARY_DIR}
        ./thirdparty/include
        ./src
)

# link with HSMCPP library (based on used dispatcher)
# ${LIB_LGPIO}
target_link_libraries(${BINARY_NAME} PRIVATE ${HSMCPP_STD_LIB} pigpio i2c gpiod)

# set compiler options for HSMCPP library (based on used dispatcher)
target_compile_options(${BINARY_NAME} PRIVATE ${HSMCPP_STD_CXX_FLAGS})




set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
if (WIN32)
    cmake_policy(SET CMP0071 NEW)
    cmake_policy(SET CMP0020 NEW)
endif()

if (CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

find_package(Qt5 COMPONENTS Core REQUIRED)
# set(PKG_CONFIG_PATH ${PKG_CONFIG_PATH};/home/ikrechetov/projects/rp4_qt/sysroot/lib/arm-linux-gnueabihf/pkgconfig)
# set(CMAKE_PREFIX_PATH "/home/ikrechetov/projects/rp4_qt/sysroot/lib/arm-linux-gnueabihf/pkgconfig")

include(FindPkgConfig)
pkg_check_modules(GLIB REQUIRED glib-2.0)
# set(GLIB_PKG_INCLUDE_DIRS
#     ${GLIB_INCLUDE_DIRS}
# )
# set(GLIB_PKG_LDFLAGS
#     ${GLIB_LDFLAGS}
# )

qt5_wrap_cpp(QT_TEST_MOC ${CMAKE_CURRENT_SOURCE_DIR}/src/qt_main.hpp)
add_executable(qtTest ${CMAKE_CURRENT_SOURCE_DIR}/src/qt_main.cpp ${QT_TEST_MOC})
target_include_directories(qtTest PUBLIC ${Qt5Core_INCLUDE_DIRS} ${CMAKE_BINARY_DIR}/src)
# qt5_use_modules(qtTest Core)
# link_directories(/home/ikrechetov/projects/rp4_qt/sysroot/lib/arm-linux-gnueabihf/)
target_link_libraries(qtTest PUBLIC Qt5::Core double-conversion gthread-2.0 pcre z ${GLIB_LDFLAGS} pthread dl)
target_compile_options(qtTest PRIVATE "-fPIC" ${HSMCPP_QT_CXX_FLAGS})

set(CMAKE_AUTOMOC OFF)
set(CMAKE_AUTORCC OFF)
set(CMAKE_AUTOUIC OFF)