/*
 * Copyright (C) 2022 Igor Krechetov
 * Distributed under MIT License. See file LICENSE for details (http://www.opensource.org/licenses/MIT)
 */
#ifndef HWIOCPP_GPIO_KEYPADMATRIX_HPP
#define HWIOCPP_GPIO_KEYPADMATRIX_HPP

#include "DeviceGPIO.hpp"
#include "utils/matrix.hpp"
#include <vector>
#include <string>
#include <map>
#include <functional>

enum class KeypadEvent
{
    UNKNOWN,
    KEY_PRESSED,
    KEY_RELEASED
};

using KeypadCallback_t = std::function<void(const KeypadEvent, const int, const int, const std::string&)>;
using KeypadKeyMap_t = std::map<std::pair<int, int>, std::string>;

class KeypadMatrix: protected DeviceGPIO
{
public:
    virtual ~KeypadMatrix();

    bool initialize(const std::vector<RP_GPIO>& rowPins, const std::vector<RP_GPIO>& colPins, const KeypadCallback_t& keyEventFunc);
    void setKeymapping(const KeypadKeyMap_t& mapping);

private:
    bool hasKeyPressed() const;

    void onPinEdgeEvent(const RP_GPIO pin, const GPIO_PIN_EDGE_EVENT event);

private:
    KeypadCallback_t mOnKeyEventCallback;
    std::vector<RP_GPIO> mRowPins;
    std::vector<RP_GPIO> mColPins;
    KeypadKeyMap_t mKeys;
    GpioPinsGroupID_t mColsGroupID;
    matrix<bool> mKeysState;
};

#endif // HWIOCPP_GPIO_KEYPADMATRIX_HPP
