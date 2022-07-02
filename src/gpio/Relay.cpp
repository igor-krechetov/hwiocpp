/*
 * Copyright (C) 2022 Igor Krechetov
 * Distributed under MIT License. See file LICENSE for details (http://www.opensource.org/licenses/MIT)
 */
#include "gpio/Relay.hpp"
#include <utils/logging.hpp>

#undef TRACE_CLASS
#define TRACE_CLASS                         "Relay"

Relay::~Relay()
{}

bool Relay::initialize(const std::vector<RP_GPIO>& pins, const std::vector<RelayNormalState>& pinStates, const bool initiallyOpen)
{
    TRACE_CALL_DEBUG_ARGS("pins=%lu, initiallyOpen=%d", pins.size(), SC2INT(initiallyOpen));
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
                    TRACE_ERROR("failed to initialize relay #%d (open=%d, gpio=%d)", i, initiallyOpen, SC2INT(mControlPins[i]));
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
