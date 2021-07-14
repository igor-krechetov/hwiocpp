#include "DeviceGPIO.hpp"
#include <hsmcpp/logging.hpp>

#undef __TRACE_CLASS__
#define __TRACE_CLASS__                         "DeviceGPIO"

std::map<std::string, DeviceGPIO::GpioChipInfo> DeviceGPIO::sOpenChips;

DeviceGPIO::~DeviceGPIO()
{
    closeDevice();
}

bool DeviceGPIO::openDevice(const RP_GPIOCHIP chip)
{
    bool result = false;

    switch(chip)
    {
        case RP_GPIOCHIP::GPIOCHIP0:
            result = openDevice("gpiochip0");
            break;
        case RP_GPIOCHIP::GPIOCHIP1:
            result = openDevice("gpiochip1");
            break;
        default:
            break;
    }

    return result;
}

bool DeviceGPIO::openDevice(const std::string& chipname)
{
    __TRACE_CALL_DEBUG__();
    bool result = false;

    if (false == isDeviceOpen())
    {
        auto itChip = sOpenChips.find(chipname);

        if (sOpenChips.end() == itChip)
        {
            GpioChipInfo newChip;

            newChip.refCount = 1;
            newChip.chip = gpiod_chip_open_by_name(chipname.c_str());

            if (nullptr != newChip.chip)
            {
                sOpenChips.insert({chipname, newChip});
                mChip = newChip.chip;
                mChipName = chipname;

                result = true;
            }
            else
            {
                __TRACE_ERROR__("Open chip failed");
            }
        }
        else
        {
            itChip->second.refCount++;
            mChip = itChip->second.chip;
            mChipName = chipname;
            result = true;
        }
    }

    return result;
}

void DeviceGPIO::closeDevice()
{
    __TRACE_CALL_DEBUG__();

    if (true == isDeviceOpen())
    {
        auto itChip = sOpenChips.find(mChipName);

        if (sOpenChips.end() != itChip)
        {
            closeAllPins();
            mChip = nullptr;
            mChipName.clear();

            itChip->second.refCount--;
            __TRACE_DEBUG__("chip <%s> ref count: %d", itChip->first.c_str(), itChip->second.refCount);

            if (itChip->second.refCount <= 0)
            {
                __TRACE_DEBUG__("closing chip <%s>", itChip->first.c_str());
                gpiod_chip_close(itChip->second.chip);
                sOpenChips.erase(itChip);
            }
        }
        else
        {
            __TRACE_ERROR__("current chip is not found. are you using DeviceGPIO from multuiple threads?");
        }
    }
}

bool DeviceGPIO::isDeviceOpen()
{
    return (nullptr != mChip);
}

bool DeviceGPIO::setPinValue(const RP_GPIO pin, const int value)
{
    __TRACE_CALL_DEBUG_ARGS__("pin=%d, value=%d", SC2INT(pin), value);
    bool result = false;
    auto itPin = mActiveLines.find(pin);

    if (itPin == mActiveLines.end())
    {
        if (true == openPin(pin, PIN_DIRECTION::OUTPUT))
        {
            itPin = mActiveLines.find(pin);
        }
    }
    else if (PIN_DIRECTION::OUTPUT != itPin->second.direction)
    {
        changePinDirection(pin, PIN_DIRECTION::OUTPUT);
    }

    if (itPin != mActiveLines.end())
    {
        result = (0 == gpiod_line_set_value(itPin->second.line, value));
    }

    __TRACE_CALL_RESULT__("%d", BOOL2INT(result));
    return result;
}

bool DeviceGPIO::getPinValue(const RP_GPIO pin, int& outValue)
{
    __TRACE_CALL_DEBUG_ARGS__("pin=%d", SC2INT(pin));
    bool result = false;
    auto itPin = mActiveLines.find(pin);

    if (itPin == mActiveLines.end())
    {
        if (true == openPin(pin, PIN_DIRECTION::INPUT))
        {
            itPin = mActiveLines.find(pin);
        }
    }
    else if (PIN_DIRECTION::INPUT != itPin->second.direction)
    {
        changePinDirection(pin, PIN_DIRECTION::INPUT);
    }

    if (itPin != mActiveLines.end())
    {
        int res = gpiod_line_get_value(itPin->second.line);

        if (res >= 0)
        {
            outValue = res;
            result = true;
        }
        else
        {
            result = false;
        }
    }

    __TRACE_CALL_RESULT__("%d (value=%d)", BOOL2INT(result), outValue);
    return result;
}

void DeviceGPIO::shiftWrite(const byte val, const RP_GPIO dataPin, const RP_GPIO clockPin, const RP_GPIO latchPin)
{
    byte mask = 0x80;
    
    // put latch down to start data sending
    setPinValue(clockPin, 0);
    setPinValue(latchPin, 0);
    setPinValue(clockPin, 1);

    // load data in reverse order
    for (int i = 0; i < 8 ; ++i)
    {
        setPinValue(clockPin, 0);
        setPinValue(dataPin, (val & mask ? 1 : 0));
        setPinValue(clockPin, 1);
        mask >>= 1;
    }

    // put latch up to store data on register
    setPinValue(clockPin, 0);
    setPinValue(latchPin, 1);
    setPinValue(clockPin, 1);
}

bool DeviceGPIO::openPin(const RP_GPIO pin, const PIN_DIRECTION direction)
{
    __TRACE_CALL_DEBUG_ARGS__("pin=%d, direction=%d", SC2INT(pin), SC2INT(direction));
    bool result = false;
    GpioLineInfo newPinInfo;

    newPinInfo.line = gpiod_chip_get_line(mChip, static_cast<int>(pin));
    newPinInfo.direction = PIN_DIRECTION::OUTPUT;

    if (nullptr != newPinInfo.line)
    {
        if (0 == gpiod_line_request_output(newPinInfo.line, "DeviceGPIO", 0))
        {
            mActiveLines.insert({pin, newPinInfo});
            result = true;
        }
        else
        {
            gpiod_line_release(newPinInfo.line);
            __TRACE_ERROR__("Request line as output failed");
        }
    }
    else
    {
        __TRACE_ERROR__("Get line failed");
    }

    return result;
}

void DeviceGPIO::closePin(const RP_GPIO pin)
{
    auto itPin = mActiveLines.find(pin);

    if (itPin != mActiveLines.end())
    {
        gpiod_line_release(itPin->second.line);
        mActiveLines.erase(itPin);
    }
}

void DeviceGPIO::closeAllPins()
{
    for (auto it = mActiveLines.begin() ; it != mActiveLines.end(); ++it)
    {
        gpiod_line_release(it->second.line);
    }

    mActiveLines.clear();
}

bool DeviceGPIO::changePinDirection(const RP_GPIO pin, const PIN_DIRECTION direction)
{
    __TRACE_CALL_DEBUG_ARGS__("pin=%d, direction=%d", SC2INT(pin), SC2INT(direction));
    bool result = false;
    auto itPin = mActiveLines.find(pin);

    if (itPin != mActiveLines.end())
    {
        if (direction != itPin->second.direction)
        {
            int res = -1;

            switch(direction)
            {
                case PIN_DIRECTION::INPUT:
                    res = gpiod_line_request_input(itPin->second.line, "DeviceGPIO");
                    break;
                case PIN_DIRECTION::OUTPUT:
                    res = gpiod_line_request_output(itPin->second.line, "DeviceGPIO", 0);
                    break;
                default:
                    break;
            }

            if (0 == res)
            {
                itPin->second.direction = direction;
                result = true;
            }
            else
            {
                __TRACE_ERROR__("changing pin direction failed");
            }
        }
        else
        {
            result = true;
        }
    }
    else
    {
        __TRACE_ERROR__("pin not open");
    }

    return result;
}