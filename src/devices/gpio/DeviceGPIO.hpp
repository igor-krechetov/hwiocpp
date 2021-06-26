#ifndef __DEVICES_I2C_DEVICEGPIO_HPP__
#define __DEVICES_I2C_DEVICEGPIO_HPP__

#include "devices/GenericDevice.hpp"
#include <string>
#include <map>
#include <gpiod.h>

// Install: sudo apt install gpiod libgpiod-dev

typedef unsigned int GpioPinId_t;

class DeviceGPIO: public GenericDevice
{
public:
    DeviceGPIO() = default;
    virtual ~DeviceGPIO();

    bool openDevice(const std::string& chipname);
    void closeDevice() override;
    bool isOpen() override;

    bool setPinValue(const GpioPinId_t pinID, const int value);
    bool getPinValue(const GpioPinId_t pinID, int& outValue);

protected:
    GpioPinId_t openPin(const unsigned int pinNumber);
    void closePin(const GpioPinId_t pinID);
    void closeAllPins();

private:
    struct gpiod_chip *mChip = nullptr;
    std::map<GpioPinId_t, struct gpiod_line *line> mActiveLines;
};

inline bool DeviceGPIO::isOpen()
{
    return mChip != nullptr;
}

#endif // __DEVICES_I2C_DEVICEGPIO_HPP__