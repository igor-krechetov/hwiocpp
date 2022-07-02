/*
 * Copyright (C) 2022 Igor Krechetov
 * Distributed under MIT License. See file LICENSE for details (http://www.opensource.org/licenses/MIT)
 */
#include "gpio/DeviceGPIO.hpp"
#include <utils/logging.hpp>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

#undef TRACE_CLASS
#define TRACE_CLASS                         "DeviceGPIO"

#define GPIO_CONSUMER_NAME                      "DeviceGPIO"

#define REG32(addr) ((volatile uint32_t *)(uintptr_t)(addr))

#define	GPIO_PERI_BASE_2711     0xFE000000
#define GPIO_BASE               (GPIO_PERI_BASE_2711 + 0x200000)
#define GPIO_2711_PULL          (GPIO_BASE + 0xe4)

#define GPPUPPDN0                57        /* Pin pull-up/down for pins 15:0  */
#define GPPUPPDN1                58        /* Pin pull-up/down for pins 31:16 */
#define GPPUPPDN2                59        /* Pin pull-up/down for pins 47:32 */
#define GPPUPPDN3                60        /* Pin pull-up/down for pins 57:48 */

#define PULLUPDN_OFFSET     37  // 0x0094 / 4
#define PULLUPDNCLK_OFFSET  38  // 0x0098 / 4

#define	BLOCK_SIZE		(4*1024)

#define PULLUPDN_OFFSET_2711_0      57
#define PULLUPDN_OFFSET_2711_1      58
#define PULLUPDN_OFFSET_2711_2      59
#define PULLUPDN_OFFSET_2711_3      60


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
    TRACE_CALL_DEBUG();
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
                TRACE_ERROR("Open chip failed");
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
    TRACE_CALL_DEBUG();

    if (true == isDeviceOpen())
    {
        auto itChip = sOpenChips.find(mChipName);

        if (sOpenChips.end() != itChip)
        {
            closeAllPins();

            if (mMonitoringThread.joinable())
            {
                TRACE_DEBUG("wait for monitoring thread to finish...");
                mMonitoringThread.join();
            }

            mChip = nullptr;
            mChipName.clear();

            itChip->second.refCount--;
            TRACE_DEBUG("chip <%s> ref count: %d", itChip->first.c_str(), itChip->second.refCount);

            if (itChip->second.refCount <= 0)
            {
                TRACE_DEBUG("closing chip <%s>", itChip->first.c_str());
                gpiod_chip_close(itChip->second.chip);
                sOpenChips.erase(itChip);
            }
        }
        else
        {
            TRACE_ERROR("current chip is not found. are you using DeviceGPIO from multiple threads?");
        }
    }
}

bool DeviceGPIO::isDeviceOpen()
{
    return (nullptr != mChip);
}

void DeviceGPIO::test2()
{
    const RP_GPIO latchPin = RP_GPIO::GPIO_21;
    const RP_GPIO clockPin = RP_GPIO::GPIO_20;
    const RP_GPIO dataPin = RP_GPIO::GPIO_16;

    setPinValue(clockPin, 0);
    setPinValue(latchPin, 0);
    setPinValue(dataPin, 0);

    setPinValue(clockPin, 1);
    wait(1000);
    setPinValue(latchPin, 1);
    wait(1000);
    setPinValue(dataPin, 1);
    wait(1000);

    setPinValue(clockPin, 0);
    setPinValue(latchPin, 0);
    setPinValue(dataPin, 0);
}

void DeviceGPIO::test()
{
    const RP_GPIO latchPin = RP_GPIO::GPIO_21;
    const RP_GPIO clockPin = RP_GPIO::GPIO_20;
    const RP_GPIO dataPin = RP_GPIO::GPIO_16;

    struct gpiod_line * line3 = gpiod_chip_find_line(mChip, "GPIO16");
    struct gpiod_line * line1 = gpiod_chip_find_line(mChip, "GPIO20");
    struct gpiod_line * line2 = gpiod_chip_find_line(mChip, "GPIO21");

    printf("GPIO16=%d\n", gpiod_line_offset(line3));
    printf("GPIO20=%d\n", gpiod_line_offset(line1));
    printf("GPIO21=%d\n", gpiod_line_offset(line2));

    printf("BEGIN (%s)\n", gpiod_version_string());

    printf("%d to LOW\n", (int)clockPin);
    // gpiod_line * line1 = gpiod_chip_get_line(mChip, static_cast<int>(clockPin));
    struct gpiod_line_request_config gpioConfig1;
    gpioConfig1.consumer = GPIO_CONSUMER_NAME;
    gpioConfig1.request_type = GPIOD_LINE_REQUEST_DIRECTION_OUTPUT;
    printf("flags1=%d\n", gpioConfig1.flags);
    // struct gpiod_line_request_config gpioConfig1 = {
	// 	.consumer = GPIO_CONSUMER_NAME,
	// 	.request_type = GPIOD_LINE_REQUEST_DIRECTION_OUTPUT,
	// };
    gpiod_line_request(line1, &gpioConfig1, 0);
    // gpiod_line_request_output(line1, GPIO_CONSUMER_NAME, 0);
    gpiod_line_set_value(line1, 0);
    gpiod_line_release(line1);
    wait(1000);

    printf("%d to LOW\n", (int)latchPin);
    // gpiod_line * line2 = gpiod_chip_get_line(mChip, static_cast<int>(latchPin));
    // struct gpiod_line_request_config gpioConfig2;
    // gpioConfig2.consumer = GPIO_CONSUMER_NAME;
    // gpioConfig2.request_type = GPIOD_LINE_REQUEST_DIRECTION_OUTPUT;
    struct gpiod_line_request_config gpioConfig2 = {
		.consumer = GPIO_CONSUMER_NAME,
		.request_type = GPIOD_LINE_REQUEST_DIRECTION_OUTPUT,
	};
    printf("flags2=%d\n", gpioConfig2.flags);
    gpiod_line_request(line2, &gpioConfig2, 0);
    // gpiod_line_request_output(line2, GPIO_CONSUMER_NAME, 0);
    gpiod_line_set_value(line2, 0);
    gpiod_line_release(line2);
    wait(1000);

    printf("%d to LOW\n", (int)dataPin);
    // gpiod_line * line3 = gpiod_chip_get_line(mChip, static_cast<int>(dataPin));
    // struct gpiod_line_request_config gpioConfig3;
    // gpioConfig3.consumer = GPIO_CONSUMER_NAME;
    // gpioConfig3.request_type = GPIOD_LINE_REQUEST_DIRECTION_OUTPUT;
    struct gpiod_line_request_config gpioConfig3 = {
		.consumer = GPIO_CONSUMER_NAME,
		.request_type = GPIOD_LINE_REQUEST_DIRECTION_OUTPUT,
	};
    printf("flags3=%d\n", gpioConfig3.flags);
    gpiod_line_request(line3, &gpioConfig3, 0);
    // gpiod_line_request_output(line3, GPIO_CONSUMER_NAME, 0);
    gpiod_line_set_value(line3, 0);

    wait(2000);

    printf("16 to HIGH\n");
    gpiod_line_set_value(line3, 1);
    printf("DONE\n");

    gpiod_line_release(line3);
}

bool DeviceGPIO::setPinValue(const RP_GPIO pin, const int value)
{
    TRACE_CALL_DEBUG_ARGS("pin=%d, value=%d", SC2INT(pin), value);
    bool result = false;

    if (true == openPin(pin, GPIO_PIN_MODE::OUTPUT))
    {
        auto itPin = mActiveLines.find(pin);

        if (itPin != mActiveLines.end())
        {
            result = (0 == gpiod_line_set_value(itPin->second.line, value));
        }
        else
        {
            TRACE_FATAL("internal data inconsistency! Has this object been used from multiple threads?");
        }
    }

    TRACE_CALL_RESULT("%d", BOOL2INT(result));
    return result;
}

bool DeviceGPIO::getPinValue(const RP_GPIO pin, int& outValue)
{
    TRACE_CALL_DEBUG_ARGS("pin=%d", SC2INT(pin));
    bool result = false;

    if (true == openPin(pin, GPIO_PIN_MODE::INPUT))
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
            TRACE_FATAL("internal data inconsistency! Has this object been used from multiple threads?");
        }
    }
    else
    {
        TRACE_ERROR("failed to open pin %d", SC2INT(pin));
    }

    TRACE_CALL_RESULT("%d (value=%d)", BOOL2INT(result), outValue);
    return result;
}

bool DeviceGPIO::setPinPullMode(const RP_GPIO pin, const GPIO_PIN_PULL pullMode)
{
    TRACE_CALL_DEBUG_ARGS("pin=%d, pullMode=%d", SC2INT(pin), SC2INT(pullMode));

    // return openPin(pin, PIN_DIRECTION::AS_IS, pullMode);
    return gpio_set_pull(static_cast<int>(pin), pullMode);
}

GpioPinsGroupID_t DeviceGPIO::registerPinsGroup(const std::vector<RP_GPIO>& pins)
{
    TRACE_CALL_DEBUG_ARGS("pins.size=%lu", pins.size());
    GpioPinsGroupID_t newGroupId = INVALID_GPIO_GROUP_ID;

    if (pins.size() > 0)
    {
        if (true == isDeviceOpen())
        {
            bool hasFailed = false;

            // open all pins. they all must be available to register a group
            for (auto itCurPin = pins.begin(); itCurPin != pins.end(); ++itCurPin)
            {
                if (false == openPin(*itCurPin, GPIO_PIN_PULL::AS_IS))
                {
                    TRACE_ERROR("failed to open pin %d", SC2INT(*itCurPin));
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

    TRACE_CALL_RESULT("%d", newGroupId);
    return newGroupId;
}

void DeviceGPIO::unregisterPinsGroup(const GpioPinsGroupID_t id)
{
    TRACE_CALL_DEBUG_ARGS("id=%d", id);
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
    TRACE_CALL_DEBUG_ARGS("id=%d", id);
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
            if (true == changeGroupDirection(itGroup->first, GPIO_PIN_MODE::OUTPUT))
            {
                if (0 == gpiod_line_set_value_bulk(&(itGroup->second), values.data()))
                {
                    result = true;
                }
                else
                {
                    TRACE_ERROR("failed to set values");
                }
            }
        }
        else
        {
            TRACE_FATAL("internal data inconsistency! Has this object been used from multiple threads?");
        }
    }
    else
    {
        TRACE_ERROR("expected %lu values, but got %lu", itPins->second.size(), values.size());
    }

    return result;
}

bool DeviceGPIO::getGroupValues(const GpioPinsGroupID_t id, std::vector<int>& outValues)
{
    TRACE_CALL_DEBUG_ARGS("id=%d", id);
    bool result = false;
    auto itGroup = mGroups.find(id);
    auto itPins = mGroupPins.find(id);

    if ((mGroups.end() != itGroup) && (itPins != mGroupPins.end()))
    {
        if (true == changeGroupDirection(itGroup->first, GPIO_PIN_MODE::INPUT))
        {
            outValues.resize(itPins->second.size());

            if (0 == gpiod_line_get_value_bulk(&(itGroup->second), outValues.data()))
            {
                // TODO: DEBUG
                for (int i = 0 ; i < outValues.size(); ++i)
                {
                    TRACE_DEBUG("[%d] = %d", i, outValues[i]);
                }

                result = true;
            }
            else
            {
                TRACE_ERROR("failed to set values");
            }
        }
    }
    else
    {
        TRACE_ERROR("group with id=%d wasnt found", id);
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
    printf("------------------------\n");
    setPinValue(latchPin, 1);
    printf("------------------------\n");
    setPinValue(clockPin, 1);
}

bool DeviceGPIO::openPin(const RP_GPIO pin, const GPIO_PIN_MODE mode, const GPIO_PIN_PULL pullMode)
{
    TRACE_CALL_DEBUG_ARGS("pin=%d, mode=%d, pullMode=%d", SC2INT(pin), SC2INT(mode), SC2INT(pullMode));
    bool result = false;
    auto itPin = mActiveLines.find(pin);

    if (itPin == mActiveLines.end())
    {
        GpioLineInfo newPinInfo;

        newPinInfo.line = gpiod_chip_get_line(mChip, static_cast<int>(pin));
        newPinInfo.mode = mode;

        if (nullptr != newPinInfo.line)
        {
            struct gpiod_line_request_config gpioConfig;

            memset(&gpioConfig, 0, sizeof(gpiod_line_request_config));
            printf("flags=%d\n", gpioConfig.flags);
            printf("request_type=%d\n", gpioConfig.request_type);
            printf("consumer=%p\n", gpioConfig.consumer);

            if (true == setPinPullMode(pin, pullMode))
            {
                TRACE_LINE();
                gpioConfig.consumer = GPIO_CONSUMER_NAME;

                if (mode != GPIO_PIN_MODE::EDGE_DETECTION)
                {
                    result = true;

                    switch(mode)
                    {
                        case GPIO_PIN_MODE::INPUT:
                            gpioConfig.request_type = GPIOD_LINE_REQUEST_DIRECTION_INPUT;
                            break;
                        case GPIO_PIN_MODE::OUTPUT:
                            gpioConfig.request_type = GPIOD_LINE_REQUEST_DIRECTION_OUTPUT;
                            break;
                        case GPIO_PIN_MODE::AS_IS:
                            gpioConfig.request_type = GPIOD_LINE_REQUEST_DIRECTION_AS_IS;
                            break;
                        case GPIO_PIN_MODE::UNKNOWN:
                        default:
                            result = false;
                            break;
                    }

                    if ((true == result) && (0 == gpiod_line_request(newPinInfo.line, &gpioConfig, 0)))
                    {
                        mActiveLines.insert({pin, newPinInfo});
                        printf("------ mActiveLines=%lu, newPinInfo.line=%p\n", mActiveLines.size(), newPinInfo.line);
                    }
                    else
                    {
                        gpiod_line_release(newPinInfo.line);
                        result = false;
                        TRACE_ERROR("Request line failed");
                    }
                }
                else
                {
                    mActiveLines.insert({pin, newPinInfo});
                    result = startEdgeEventsMonitorining(pin);

                    if (false == result)
                    {
                        closePin(pin);
                    }
                }
            }
        }
        else
        {
            TRACE_ERROR("Get line failed");
        }
    }
    else
    {
        result = true;

        if ((GPIO_PIN_MODE::AS_IS != mode) && (mode != itPin->second.mode))
        {
            printf("---- 1\n");
            result = changePinDirection(pin, mode);
        }

        if ((GPIO_PIN_PULL::AS_IS != pullMode) && (pullMode != itPin->second.pull))
        {
            printf("---- 2\n");
            result = changePinPullMode(pin, pullMode);
        }
    }

    return result;
}

bool DeviceGPIO::openPin(const RP_GPIO pin, const GPIO_PIN_PULL pullMode)
{
    TRACE_CALL_DEBUG_ARGS("pin=%d, pullMode=%d", SC2INT(pin), SC2INT(pullMode));
    bool result = false;
    auto itPin = mActiveLines.find(pin);

    if (itPin != mActiveLines.end())
    {
        closePin(pin);
        itPin = mActiveLines.find(pin);
    }

    if (itPin == mActiveLines.end())
    {
        GpioLineInfo newPinInfo;

        newPinInfo.line = gpiod_chip_get_line(mChip, static_cast<int>(pin));
        
        if (nullptr != newPinInfo.line)
        {
            newPinInfo.mode = GPIO_PIN_MODE::AS_IS;
            newPinInfo.pull = pullMode;

            result = setPinPullMode(pin, pullMode);

            if (true == result)
            {
                mActiveLines.insert({pin, newPinInfo});
            }
        }
        else
        {
            TRACE_ERROR("Get line failed");
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

void DeviceGPIO::registerEdgeEventsCallback(const EdgeEventCallback_t& callback)
{
    mEdgeCallback = callback;
}

void DeviceGPIO::unregisterEdgeEventsCallback()
{
    mEdgeCallback = nullptr;

    // if (true == mMonitoringThread.joinable())
    // {
    //     mMonitoringThread.join();
    // }
}

bool DeviceGPIO::startEdgeEventsMonitorining(const RP_GPIO pin)
{
    bool result = false;

    if (mEdgeCallback)
    {
        TRACE_CALL_ARGS("pin=%d (v2)", SC2INT(pin));
        auto itPin = mActiveLines.find(pin);

        if (itPin != mActiveLines.end())
        {
            if (0 == gpiod_line_request_both_edges_events(itPin->second.line, GPIO_CONSUMER_NAME))
            {
                if ((false == mIsMonitoring) && (true == mMonitoringThread.joinable()))
                {
                    mMonitoringThread.join();
                }

                if (false == mMonitoringThread.joinable())
                {
                    mMonitoringThread = std::thread(std::bind(&DeviceGPIO::threadEdgeMonitoring, this));
                }

                result = true;
            }
            else 
            {
                TRACE_ERROR("gpiod_line_request_bulk_both_edges_events failed");
            }
        }
    }

    return result;
}

void DeviceGPIO::startEdgeEventsMonitorining(const GpioPinsGroupID_t groupID)
{
    TRACE_CALL_ARGS("groupID=%d", SC2INT(groupID));
    
    if (mEdgeCallback)
    {
        auto itGroup = mGroupPins.find(groupID);

        if (itGroup != mGroupPins.end())
        {
            for (RP_GPIO curPin: itGroup->second)
            {
                startEdgeEventsMonitorining(curPin);
            }
        }
    }
}

// void DeviceGPIO::stopEdgeEventsMonitorining(const RP_GPIO pin)
// {
//     // TODO impl
// }

// void DeviceGPIO::stopEdgeEventsMonitorining(const GpioPinsGroupID_t groupID)
// {
//     // TODO impl
// }

//==============================================================================================================================
// protected
bool DeviceGPIO::changePinDirection(const RP_GPIO pin, const GPIO_PIN_MODE direction)
{
    TRACE_CALL_DEBUG_ARGS("pin=%d, direction=%d", SC2INT(pin), SC2INT(direction));
    bool result = false;
    auto itPin = mActiveLines.find(pin);

    if (itPin != mActiveLines.end())
    {
        if (direction != itPin->second.mode)
        {
            int res = -1;

            gpiod_line_release(itPin->second.line);

            switch(direction)
            {
                case GPIO_PIN_MODE::INPUT:
                    res = gpiod_line_request_input(itPin->second.line, GPIO_CONSUMER_NAME);
                    break;
                case GPIO_PIN_MODE::OUTPUT:
                    res = gpiod_line_request_output(itPin->second.line, GPIO_CONSUMER_NAME, 0);
                    break;
                default:
                    break;
            }

            if (0 == res)
            {
                itPin->second.mode = direction;
                result = true;
            }
            else
            {
                TRACE_ERROR("changing pin direction failed. closingPin");
                closePin(pin);
            }
        }
        else
        {
            result = true;
        }
    }
    else
    {
        TRACE_ERROR("pin not open");
    }

    return result;
}

bool DeviceGPIO::changePinPullMode(const RP_GPIO pin, const GPIO_PIN_PULL pullMode)
{
    TRACE_CALL_DEBUG_ARGS("pin=%d, pullMode=%d", SC2INT(pin), SC2INT(pullMode));
    bool result = false;
    auto itPin = mActiveLines.find(pin);

    if (itPin != mActiveLines.end())
    {
        if ((GPIO_PIN_PULL::AS_IS != pullMode) && (pullMode != itPin->second.pull))
        {
            // int flags = 0;

            // switch(pullMode)
            // {
            //     // case GPIO_PIN_PULL::DISABLE:
            //     //     flags = GPIOD_LINE_REQUEST_FLAG_BIAS_DISABLE;
            //     //     break;
            //     // case GPIO_PIN_PULL::PULL_DOWN:
            //     //     flags = GPIOD_LINE_REQUEST_FLAG_BIAS_PULL_DOWN;
            //     //     break;
            //     // case GPIO_PIN_PULL::PULL_UP:
            //     //     flags = GPIOD_LINE_REQUEST_FLAG_BIAS_PULL_UP;
            //     //     break;
            //     default:
            //         break;
            // }

            if (true == setPinPullMode(pin, pullMode))
            {
                itPin->second.pull = pullMode;
                result = true;
            }
            else
            {
                TRACE_ERROR("changing pin pull mode failed");
            }
        }
        else
        {
            result = true;
        }
    }
    else
    {
        TRACE_ERROR("pin not open");
    }

    return result;
}

bool DeviceGPIO::changeGroupDirection(const GpioPinsGroupID_t id, const GPIO_PIN_MODE direction)
{
    TRACE_CALL_DEBUG_ARGS("pin=%d, direction=%d", id, SC2INT(direction));
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

                if (direction != itPinInfo->second.mode)
                {
                    gpiod_line_release_bulk(&(itGroup->second));

                    switch(direction)
                    {
                        case GPIO_PIN_MODE::INPUT:
                            res = gpiod_line_request_bulk_input(&(itGroup->second), GPIO_CONSUMER_NAME);
                            break;
                        case GPIO_PIN_MODE::OUTPUT:
                        {
                            std::vector<int> defValues(itPins->second.size(), 0);
                            res = gpiod_line_request_bulk_output(&(itGroup->second), GPIO_CONSUMER_NAME, defValues.data());
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

                                itPinInfo->second.mode = direction;
                            }
                        }

                        result = true;
                    }
                    else
                    {
                        TRACE_ERROR("changing pins group direction failed");
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
        TRACE_ERROR("group with id %d was not found", id);
    }

    return result;
}

int DeviceGPIO::gpio_get_pull(unsigned int nr)
{
    unsigned int offset = (nr % 16) * 2;
    volatile uint32_t *pull_cfg = REG32(GPIO_2711_PULL);
    return (pull_cfg[nr / 16] >> offset) & 0x3;
}

void printTitle(int b, int e)
{
    for(int i = e ; i >= b ; i--)
    {
        printf(" %.2d", i);
    }
    printf("\n");
}

void dumpInt(unsigned int val)
{
    unsigned int mask = 1 << 31;

    for (int i = 0 ; i < 32 ; i++)
    {
        if (i % 2 == 0)
        {
            printf(" ");
        }

        printf("%d", (int)(bool)((val & mask) != 0));
        mask = mask >> 1;
    }
    printf("\n");
}

void gpio_dump_mem()
{
    int fd = open("/dev/gpiomem", O_RDWR | O_SYNC);// | O_CLOEXEC);

    if (fd >= 0)
    {
        // 0001 0101
        uint32_t* gpio_base = reinterpret_cast<uint32_t*>( mmap(0, BLOCK_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0) );

        int is2711 = *(gpio_base + PULLUPDN_OFFSET_2711_3) != 0x6770696f;

        if (is2711)
        {
            printTitle(0, 15);
            dumpInt(*(gpio_base + GPPUPPDN0));
            printf("\n");

            printTitle(16, 31);
            dumpInt(*(gpio_base + GPPUPPDN1));
            printf("\n");

            printTitle(32, 47);
            dumpInt(*(gpio_base + GPPUPPDN2));
            printf("\n");

            printTitle(48, 63);
            dumpInt(*(gpio_base + GPPUPPDN3));
            printf("\n");
        }
        else
        {
            printf("Not 2711\n");
        }

        munmap(gpio_base, BLOCK_SIZE);
    }
    close(fd);
}

// NOTE: for 2711 only
bool DeviceGPIO::gpio_set_pull(const int gpio, const GPIO_PIN_PULL type)
{
    bool result = false;

    if (type != GPIO_PIN_PULL::AS_IS)
    {
        // gpio_dump_mem();
    
        // src: https://www.raspberrypi.org/forums/viewtopic.php?t=264691
        // src: https://github.com/WiringPi/WiringPi/blob/7f8fe26e4f775abfced43c07657a353f03ddb2d0/wiringPi/wiringPi.c
        /* 2711 has a different mechanism for pin pull-up/down/enable  */
        int fd = open("/dev/gpiomem", O_RDWR | O_SYNC);// | O_CLOEXEC);

        if (fd >= 0)
        {
            // 0001 0101
            uint32_t* gpio_base = reinterpret_cast<uint32_t*>( mmap(0, BLOCK_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0) );

            int is2711 = *(gpio_base + PULLUPDN_OFFSET_2711_3) != 0x6770696f;

            if (is2711)
            {
                int pullreg = GPPUPPDN0 + (gpio >> 4);// +1
                int pullshift = (gpio & 0xF) << 1;// 0000 0101 -> 0000 1010 (10)
                unsigned int pullbits;
                unsigned int pull;

                switch (type)
                {
                    case GPIO_PIN_PULL::DISABLE:
                        pull = 0;
                        break;
                    case GPIO_PIN_PULL::PULL_UP:
                        pull = 1;
                        break;
                    case GPIO_PIN_PULL::PULL_DOWN:
                        pull = 2;
                        break;
                    default:
                        return false; /* An illegal value */
                }

                pullbits = *(gpio_base + pullreg);// base + 58

                // set bits for our pin to 0
                pullbits &= ~(3 << pullshift); // 3 << 10 = 1100 0000 0000
                // apply pul mode to pin
                pullbits |= (pull << pullshift);
                // write new value to memory
                *(gpio_base + pullreg) = pullbits;

                result = true;
            }
            else
            {
                printf("Not 2711\n");
            }

            munmap(gpio_base, BLOCK_SIZE);
        }

        close(fd);
    }
    else
    {
        result = true;
    }

    return result;
}

/*

          VUTSRQPONMLKJIHGFEDCBA9876543210

GPPUPPDN0 ..............................xx GPIOO
GPPUPPDN0 ............................xx.. GPIO1
GPPUPPDN0 ..........................xx.... GPIO2
GPPUPPDN0 ........................xx...... GPIO3
GPPUPPDN0 ......................xx........ GPIO4
GPPUPPDN0 ....................xx.......... GPIO5
GPPUPPDN0 ..................xx............ GPIO6
GPPUPPDN0 ................xx.............. GPIO7
GPPUPPDN0 ..............xx................ GPIO8
GPPUPPDN0 ............xx.................. GPIO9
GPPUPPDN0 ..........xx.................... GPIO10
GPPUPPDN0 ........xx...................... GPIO11
GPPUPPDN0 ......xx........................ GPIO12
GPPUPPDN0 ....xx.......................... GPIO13
GPPUPPDN0 ..xx............................ GPIO14
GPPUPPDN0 xx.............................. GPIO15

GPPUPPDN1 ..............................xx GPIO16
GPPUPPDN1 ............................xx.. GPIO17
GPPUPPDN1 ..........................xx.... GPIO18
GPPUPPDN1 ........................xx...... GPIO19
GPPUPPDN1 ......................xx........ GPIO20
GPPUPPDN1 ....................xx.......... GPIO21
GPPUPPDN1 ..................xx............ GPIO22
GPPUPPDN1 ................xx.............. GPIO23
GPPUPPDN1 ..............xx................ GPIO24
GPPUPPDN1 ............xx.................. GPIO25
GPPUPPDN1 ..........xx.................... GPIO26
GPPUPPDN1 ........xx...................... GPIO27
GPPUPPDN1 ......xx........................ GPIO28
GPPUPPDN1 ....xx.......................... GPIO29
GPPUPPDN1 ..xx............................ GPIO30
GPPUPPDN1 xx.............................. GPIO31

*/

void DeviceGPIO::threadEdgeMonitoring()
{
    TRACE_CALL();

    mIsMonitoring = true;

    while(true)
    {
        struct gpiod_line_bulk monitoringBulk = GPIOD_LINE_BULK_INITIALIZER;

        for (auto it = mActiveLines.begin(); it != mActiveLines.end(); ++it)
        {
            if (GPIO_PIN_MODE::EDGE_DETECTION == it->second.mode)
            {
                gpiod_line_bulk_add(&monitoringBulk, it->second.line);
            }
        }

        if ((gpiod_line_bulk_num_lines(&monitoringBulk) > 0) && (nullptr != mEdgeCallback))
        {
            struct timespec timeout;
            struct gpiod_line_bulk eventBulk;

            timeout.tv_sec = 5;
            timeout.tv_nsec = 0;

            int rc = gpiod_line_event_wait_bulk(&monitoringBulk, &timeout, &eventBulk);

            if (rc > 0)
            {
                const unsigned int linesCount = gpiod_line_bulk_num_lines(&eventBulk);

                for (unsigned int i = 0 ; i < linesCount; ++i)
                {
                    gpiod_line* curLine = gpiod_line_bulk_get_line(&eventBulk, i);
                    RP_GPIO pin = static_cast<RP_GPIO>(gpiod_line_offset(curLine));
                    gpiod_line_event eventInfo;

                    if (0 == gpiod_line_event_read(curLine, &eventInfo))
                    {
                        TRACE_DEBUG("pin=%d, event=%d", SC2INT(pin), eventInfo.event_type);
                        GPIO_PIN_EDGE_EVENT event = GPIO_PIN_EDGE_EVENT::UNKNOWN;

                        switch(eventInfo.event_type)
                        {
                            case GPIOD_LINE_EVENT_RISING_EDGE:
                                event = GPIO_PIN_EDGE_EVENT::RISING_EDGE;
                                break;
                            case GPIOD_LINE_EVENT_FALLING_EDGE:
                                event = GPIO_PIN_EDGE_EVENT::FALLING_EDGE;
                                break;
                        }
                        
                        mEdgeCallback(pin, event);
                    }
                }
            }
            else if (0 < rc)
            {
                TRACE_ERROR("gpiod_line_event_wait_bulk -> error");
            }
        }
        else
        {
            TRACE_DEBUG("no pins to monitor - exit thread");
            break;
        }
    }

    mIsMonitoring = false;
}