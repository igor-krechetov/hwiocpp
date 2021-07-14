#include "Relay.hpp"
#include <hsmcpp/logging.hpp>

#undef __TRACE_CLASS__
#define __TRACE_CLASS__                         "Relay"

Relay::~Relay()
{}

bool Relay::initialize(const std::vector<RP_GPIO>& pins, const std::vector<RelayNormalState>& pinStates, const bool initiallyOpen)
{
    __TRACE_CALL_DEBUG_ARGS__("pins=%lu, initiallyOpen=%d", pins.size(), SC2INT(initiallyOpen));
    bool result = false;
    
    if ((pins.empty() == false) && (pins.size() == pinStates.size()))
    {
        if (true == openDevice())
        {
            mControlPins = pins;
            mPinNormalStates = pinStates;

            for (int i = 0 ; i < mControlPins.size(); ++i)
            {
                result = setRelayValue(i, initiallyOpen);

                if (false == result)
                {
                    __TRACE_ERROR__("failed to initialize relay #%d (open=%d, gpio=%d)", i, initiallyOpen, SC2INT(mControlPins[i]));
                    closeDevice();
                    mControlPins.clear();
                    break;
                }
            }
        }
    }

    return result;
}

bool Relay::openRelay(const int index)
{
    return setRelayValue(index, true);
}

bool Relay::closeRelay(const int index)
{
    return setRelayValue(index, false);
}

bool Relay::setRelayValue(const int index, const bool valueOpen)
{
    bool result = false;

    if ((index >= 0 && index < mControlPins.size()) && (true == isDeviceOpen()))
    {
        int newValue = 0;

        if (RelayNormalState::NORMALLY_OPEN == mPinNormalStates[index])
        {
            newValue = (valueOpen ? 1 : 0);
        }
        else
        {
            newValue = (valueOpen ? 0 : 1);
        }

        result = setPinValue(mControlPins[index], newValue);
    }

    return result;
}
