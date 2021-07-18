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

    if (true == openPin(pin, PIN_DIRECTION::OUTPUT))
    {
        auto itPin = mActiveLines.find(pin);

        if (itPin != mActiveLines.end())
        {
            result = (0 == gpiod_line_set_value(itPin->second.line, value));
        }
        else
        {
            __TRACE_FATAL__("internal data inconsistency! Has this object been used from multiple threads?");
        }
    }

    __TRACE_CALL_RESULT__("%d", BOOL2INT(result));
    return result;
}

bool DeviceGPIO::getPinValue(const RP_GPIO pin, int& outValue)
{
    __TRACE_CALL_DEBUG_ARGS__("pin=%d", SC2INT(pin));
    bool result = false;

    if (true == openPin(pin, PIN_DIRECTION::INPUT))
    {
        auto itPin = mActiveLines.find(pin);

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
        else
        {
            __TRACE_FATAL__("internal data inconsistency! Has this object been used from multiple threads?");
        }
    }
    else
    {
        __TRACE_ERROR__("failed to open pin %d", SC2INT(pin));
    }

    __TRACE_CALL_RESULT__("%d (value=%d)", BOOL2INT(result), outValue);
    return result;
}

GpioPinsGroupID_t DeviceGPIO::registerPinsGroup(const std::vector<RP_GPIO>& pins)
{
    __TRACE_CALL_DEBUG_ARGS__("pins.size=%lu", pins.size());
    GpioPinsGroupID_t newGroupId = INVALID_GPIO_GROUP_ID;

    if (pins.size() > 0)
    {
        if (true == isDeviceOpen())
        {
            bool hasFailed = false;

            // open all pins. they all must be available to register a group
            for (auto itCurPin = pins.begin(); itCurPin != pins.end(); ++itCurPin)
            {
                if (false == openPin(*itCurPin, PIN_DIRECTION::UNKNOWN))
                {
                    __TRACE_ERROR__("failed to open pin %d", SC2INT(*itCurPin));
                    hasFailed = true;
                    break;
                }
            }

            if (false == hasFailed)
            {
                // Create and init lines bulk
                struct gpiod_line_bulk groupBulk = GPIOD_LINE_BULK_INITIALIZER;
                
                gpiod_line_bulk_init(&groupBulk);

                for (auto itCurPin = pins.begin(); itCurPin != pins.end(); ++itCurPin)
                {
                    auto itPinInfo = mActiveLines.find(*itCurPin);

                    // no need to check because openPin() is supposed to make sure pin is available in mActiveLines
                    gpiod_line_bulk_add(&groupBulk, itPinInfo->second.line);
                }

                newGroupId = mNextID++;
                mGroupPins.emplace(newGroupId, pins);
                mGroups.emplace(newGroupId, groupBulk);
            }
            else
            {
                // close group pins if we failed to open all of them
                for (auto itCurPin = pins.begin(); itCurPin != pins.end(); ++itCurPin)
                {
                    closePin(*itCurPin);
                }
            }
        }
    }

    __TRACE_CALL_RESULT__("%d", newGroupId);
    return newGroupId;
}

void DeviceGPIO::unregisterPinsGroup(const GpioPinsGroupID_t id)
{
    __TRACE_CALL_DEBUG_ARGS__("id=%d", id);
    auto itGroup = mGroupPins.find(id);

    if (mGroupPins.end() != itGroup)
    {
        for (auto itCurPin = itGroup->second.begin(); itCurPin != itGroup->second.end(); ++itCurPin)
        {
            closePin(*itCurPin);
        }

        mGroupPins.erase(itGroup);
        mGroups.erase(id);
    }
}

bool DeviceGPIO::setGroupValues(const GpioPinsGroupID_t id, const std::vector<int>& values)
{
    __TRACE_CALL_DEBUG_ARGS__("id=%d", id);
    bool result = false;
    auto itPins = mGroupPins.find(id);

    // for (auto it: values)
    // {
    //     printf("%d ", it);
    // }
    // printf("\n");

    if ((itPins != mGroupPins.end()) && (itPins->second.size() == values.size()))
    {
        auto itGroup = mGroups.find(id);

        if (mGroups.end() != itGroup)
        {
            if (true == changeGroupDirection(itGroup->first, PIN_DIRECTION::OUTPUT))
            {
                if (0 == gpiod_line_set_value_bulk(&(itGroup->second), values.data()))
                {
                    result = true;
                }
                else
                {
                    __TRACE_ERROR__("failed to set values");
                }
            }
        }
        else
        {
            __TRACE_FATAL__("internal data inconsistency! Has this object been used from multiple threads?");
        }
    }
    else
    {
        __TRACE_ERROR__("expected %lu values, but got %lu", itPins->second.size(), values.size());
    }

    return result;
}

bool DeviceGPIO::getGroupValues(const GpioPinsGroupID_t id, std::vector<int>& outValues)
{
    __TRACE_CALL_DEBUG_ARGS__("id=%d", id);
    bool result = false;
    auto itGroup = mGroups.find(id);
    auto itPins = mGroupPins.find(id);

    if ((mGroups.end() != itGroup) && (itPins != mGroupPins.end()))
    {
        if (true == changeGroupDirection(itGroup->first, PIN_DIRECTION::INPUT))
        {
            outValues.resize(itPins->second.size());

            if (0 == gpiod_line_get_value_bulk(&(itGroup->second), outValues.data()))
            {
                result = true;
            }
            else
            {
                __TRACE_ERROR__("failed to set values");
            }
        }
    }
    else
    {
        __TRACE_ERROR__("group with id=%d wasnt found", id);
    }

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
    auto itPin = mActiveLines.find(pin);

    if (itPin == mActiveLines.end())
    {
        GpioLineInfo newPinInfo;

        newPinInfo.line = gpiod_chip_get_line(mChip, static_cast<int>(pin));
        newPinInfo.direction = direction;

        if (nullptr != newPinInfo.line)
        {
            int res = -1;

            switch(direction)
            {
                case PIN_DIRECTION::INPUT:
                    res = gpiod_line_request_input(newPinInfo.line, "DeviceGPIO");
                    break;
                case PIN_DIRECTION::OUTPUT:
                    res = gpiod_line_request_output(newPinInfo.line, "DeviceGPIO", 0);
                    break;
                case PIN_DIRECTION::UNKNOWN:
                    res = 0;
                    break;
                default:
                    break;
            }

            if (0 == res)
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
    }
    else
    {
        if (direction != itPin->second.direction)
        {
            result = changePinDirection(pin, direction);
        }
        else
        {
            result = true;
        }
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

bool DeviceGPIO::changeGroupDirection(const GpioPinsGroupID_t id, const PIN_DIRECTION direction)
{
    __TRACE_CALL_DEBUG_ARGS__("pin=%d, direction=%d", id, SC2INT(direction));
    bool result = false;
    auto itGroup = mGroups.find(id);

    if (itGroup != mGroups.end())
    {
        auto itPins = mGroupPins.find(id);

        if (mGroupPins.end() != itPins)
        {
            auto itPinInfo = mActiveLines.find(itPins->second.front());

            if (mActiveLines.end() != itPinInfo)
            {
                int res = -1;

                if (direction != itPinInfo->second.direction)
                {
                    switch(direction)
                    {
                        case PIN_DIRECTION::INPUT:
                            res = gpiod_line_request_bulk_input(&(itGroup->second), "DeviceGPIO");
                            break;
                        case PIN_DIRECTION::OUTPUT:
                        {
                            std::vector<int> defValues(itPins->second.size(), 0);
                            res = gpiod_line_request_bulk_output(&(itGroup->second), "DeviceGPIO", defValues.data());
                            break;
                        }
                        default:
                            break;
                    }

                    if (0 == res)
                    {
                        auto itPins = mGroupPins.find(id);

                        if (mGroupPins.end() != itPins)
                        {
                            for (auto itCurPin = itPins->second.begin(); itCurPin != itPins->second.end(); ++itCurPin)
                            {
                                auto itPinInfo = mActiveLines.find(*itCurPin);

                                itPinInfo->second.direction = direction;
                            }
                        }

                        result = true;
                    }
                    else
                    {
                        __TRACE_ERROR__("changing pins group direction failed");
                    }
                }
                else
                {
                    result = true;
                }
            }
        }
    }
    else
    {
        __TRACE_ERROR__("group with id %d was not found", id);
    }

    return result;
}