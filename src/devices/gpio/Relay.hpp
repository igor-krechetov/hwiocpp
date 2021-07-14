#ifndef __DEVICES_GPIO_RELAY_HPP__
#define __DEVICES_GPIO_RELAY_HPP__

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

#endif // __DEVICES_GPIO_RELAY_HPP__