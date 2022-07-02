/*
 * Copyright (C) 2022 Igor Krechetov
 * Distributed under MIT License. See file LICENSE for details (http://www.opensource.org/licenses/MIT)
 */
#ifndef HWIOCPP_GPIO_RELAY_HPP
#define HWIOCPP_GPIO_RELAY_HPP

#include "DeviceGPIO.hpp"
#include <vector>

enum class RelayNormalState
{
    NORMALLY_OPEN,
    NORMALLY_CLOSED
};

class Relay: protected DeviceGPIO
{
public:
    virtual ~Relay();

    bool initialize(const std::vector<RP_GPIO>& pins, const std::vector<RelayNormalState>& pinStates, const bool initiallyOpen = true);
    bool openRelay(const int index);
    bool closeRelay(const int index);
    bool setRelayValue(const int index, const bool closed);

private:
    std::vector<RP_GPIO> mControlPins;
    std::vector<RelayNormalState> mPinNormalStates;
};

#endif // HWIOCPP_GPIO_RELAY_HPP