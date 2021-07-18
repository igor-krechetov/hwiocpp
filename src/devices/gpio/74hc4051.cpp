#include "74hc4051.hpp"
#include <hsmcpp/logging.hpp>

#undef __TRACE_CLASS__
#define __TRACE_CLASS__                         "Dev74HC4051"

bool Dev74HC4051::initialize(const RP_GPIO pinA,
                             const RP_GPIO pinB, 
                             const RP_GPIO pinC, 
                             const unsigned int initialChannel, 
                             const RP_GPIO pinInhibitor)
{
    __TRACE_CALL_DEBUG_ARGS__("pinA=%d, pinB=%d, pinC=%d, initialChannel=%u, pinInhibitor=%d",
                              SC2INT(pinA), SC2INT(pinB), SC2INT(pinC), initialChannel, SC2INT(pinInhibitor));
    bool result = false;

    if (true == openDevice())
    {
        mPinInhibitor = pinInhibitor;
        mPinsGroup = registerPinsGroup({pinC, pinB, pinA});
        selectChannel(initialChannel);
        result = true;
    }

    return result;
}

bool Dev74HC4051::selectChannel(const unsigned int channel)
{
    __TRACE_CALL_DEBUG_ARGS__("channel=%u", channel);
    bool result = false;

    if ((true == isDeviceOpen()) && (channel <= DEV_74HC4051_MAX_CHANNELS))
    {
        //                                 C  B  A
        const std::vector<int> vals[8] = {{0, 0, 0},    // X0
                                          {0, 0, 1},    // X1
                                          {0, 1, 0},    // X2
                                          {0, 1, 1},    // X3
                                          {1, 0, 0},    // X4
                                          {1, 0, 1},    // X5
                                          {1, 1, 0},    // X6
                                          {1, 1, 1}};   // X7

        result = DeviceGPIO::setGroupValues(mPinsGroup, vals[channel]);

        if (true == result)
        {
            mCurrentChannel = channel;
        }
    }

    return result;
}

bool Dev74HC4051::disableOutput(const bool disable)
{
    __TRACE_CALL_DEBUG_ARGS__("disable=%d", BOOL2INT(disable));
    bool result = false;

    if ((RP_GPIO::UNKNOWN != mPinInhibitor) && (true == isDeviceOpen()))
    {
        setPinValue(mPinInhibitor, (true == disable? 1 : 0));
    }

    return result;
}