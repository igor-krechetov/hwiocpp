#ifndef __DEVICES_GPIO_DEVICEGPIO_HPP__
#define __DEVICES_GPIO_DEVICEGPIO_HPP__

#include "devices/GenericDevice.hpp"
#include <string>
#include <map>
#include <memory>
#include <gpiod.h>

// doc: https://git.kernel.org/pub/scm/libs/libgpiod/libgpiod.git/tree/include/gpiod.h
// examples: https://github.com/starnight/libgpiod-example/
// Install: sudo apt install gpiod libgpiod-dev

enum class RP_GPIOCHIP
{
    UNKNOWN,
    GPIOCHIP0,
    GPIOCHIP1
};

enum class RP_GPIO
{
    // gpiochip0
    GPIO_00 = 0,
    GPIO_01 = 1,
    GPIO_02 = 2,
    GPIO_03 = 3,
    GPIO_04 = 4,
    GPIO_05 = 5,
    GPIO_06 = 6,
    GPIO_07 = 7,
    GPIO_08 = 8,
    GPIO_09 = 9,
    GPIO_10 = 10,
    GPIO_11 = 11,
    GPIO_12 = 12,
    GPIO_13 = 13,
    GPIO_14 = 14,
    GPIO_15 = 15,
    GPIO_16 = 16,
    GPIO_17 = 17,
    GPIO_18 = 18,
    GPIO_19 = 19,
    GPIO_20 = 20,
    GPIO_21 = 21,
    GPIO_22 = 22,
    GPIO_23 = 23,
    GPIO_24 = 24,
    GPIO_25 = 25,
    GPIO_26 = 26,
    GPIO_27 = 27,

    ID_SDA = 0,
    ID_SCL = 1,
    SDA1 = 2,
    SCL1 = 3,
    GPIO_GCLK = 4,
    SPI_CE1_N = 7,
    SPI_CE0_N = 8,
    SPI_MISO = 9,
    SPI_MOSI = 10,
    SPI_SCLK = 11,
    TXD1 = 14,
    RXD1 = 15,
    RGMII_MDIO = 28,
    RGMIO_MDC = 29,
    CTS0 = 30,
    RTS0 = 31,
    TXD0 = 32,
    RXD0 = 33,
    SD1_CLK = 34,
    SD1_CMD = 35,
    SD1_DATA0 = 36,
    SD1_DATA1 = 37,
    SD1_DATA2 = 38,
    SD1_DATA3 = 39,
    PWM0_MISO = 40,
    PWM1_MOSI = 41,
    STATUS_LED_G_CLK = 42,
    SPIFLASH_CE_N = 43,
    SDA0 = 44,
    SCL0 = 45,
    RGMII_RXCLK = 46,
    RGMII_RXCTL = 47,
    RGMII_RXD0 = 48,
    RGMII_RXD1 = 49,
    RGMII_RXD2 = 50,
    RGMII_RXD3 = 51,
    RGMII_TXCLK = 52,
    RGMII_TXCTL = 53,
    RGMII_TXD0 = 54,
    RGMII_TXD1 = 55,
    RGMII_TXD2 = 56,
    RGMII_TXD3 = 57,

    // gpiochip1
    BT_ON = 0,
    WL_ON = 1,
    PWR_LED_OFF = 2,
    GLOBAL_RESET = 3,
    VDD_SD_IO_SEL = 4,
    CAM_GPIO = 5,
    SD_PWR_ON = 6,
    SD_OC_N = 7
};

class DeviceGPIO: public GenericDevice
{
    enum class PIN_DIRECTION
    {
        UNKNOWN = 0,
        INPUT = 1,
        OUTPUT = 2
    };

    struct GpioChipInfo
    {
        struct gpiod_chip* chip = nullptr;
        // RP_GPIOCHIP type = RP_GPIOCHIP::UNKNOWN;
        int refCount = 0;
    };

    struct GpioLineInfo
    {
        struct gpiod_line* line = nullptr;
        PIN_DIRECTION direction = PIN_DIRECTION::UNKNOWN;
    };

public:
    DeviceGPIO() = default;
    virtual ~DeviceGPIO();

    bool openDevice(const RP_GPIOCHIP chip = RP_GPIOCHIP::GPIOCHIP0);
    bool openDevice(const std::string& chipname);
    void closeDevice() override;
    bool isDeviceOpen() override;

    bool setPinValue(const RP_GPIO pin, const int value);
    bool getPinValue(const RP_GPIO pin, int& outValue);

    void shiftWrite(const byte value, const RP_GPIO dataPin, const RP_GPIO clockPin, const RP_GPIO latchPin);

protected:
    bool openPin(const RP_GPIO pin, const PIN_DIRECTION direction);
    void closePin(const RP_GPIO pin);
    void closeAllPins();
    bool changePinDirection(const RP_GPIO pin, const PIN_DIRECTION direction);

private:
    static std::map<std::string, GpioChipInfo> sOpenChips;

    std::string mChipName;
    struct gpiod_chip *mChip = nullptr;
    std::map<RP_GPIO, GpioLineInfo> mActiveLines;
};

#endif // __DEVICES_GPIO_DEVICEGPIO_HPP__