/*
 * Copyright (C) 2022 Igor Krechetov
 * Distributed under MIT License. See file LICENSE for details (http://www.opensource.org/licenses/MIT)
 */
#include "gpio/KeypadMatrix.hpp"
#include <utils/logging.hpp>
#include <algorithm>

#undef TRACE_CLASS
#define TRACE_CLASS                         "KeypadMatrix"

KeypadMatrix::~KeypadMatrix()
{
}

bool KeypadMatrix::initialize(const std::vector<RP_GPIO>& rowPins, const std::vector<RP_GPIO>& colPins, const KeypadCallback_t& keyEventFunc)
{
    TRACE_CALL_DEBUG_ARGS("rowPins=%lu, colPins=%lu", rowPins.size(), colPins.size());
    bool result = false;
    
    if ((false == isDeviceOpen()) && (true == openDevice()))
    {
        registerEdgeEventsCallback(std::bind(&KeypadMatrix::onPinEdgeEvent, this, std::placeholders::_1, std::placeholders::_2));
        mRowPins = rowPins;
        mColPins = colPins;
        mOnKeyEventCallback = keyEventFunc;
        mKeysState = matrix<bool>(mColPins.size(), mRowPins.size(), false);

        mColsGroupID = registerPinsGroup(mColPins);
        std::vector<int> colValuesLow(mColPins.size(), 0);

        setGroupValues(mColsGroupID, colValuesLow);

        for (RP_GPIO curColPin: mColPins)
        {
            openPin(curColPin, GPIO_PIN_MODE::AS_IS, GPIO_PIN_PULL::PULL_DOWN);
        }

        for (RP_GPIO curRowPin: mRowPins)
        {
            openPin(curRowPin, GPIO_PIN_MODE::EDGE_DETECTION, GPIO_PIN_PULL::PULL_UP);
        }

        result = true;
    }

    return result;
}

void KeypadMatrix::setKeymapping(const KeypadKeyMap_t& mapping)
{
    mKeys = mapping;
}

void dumpMatrix(const matrix<bool>& m)
{
    // TODO
    for (size_t r = 0 ; r < m.height(); ++r)
    {
        for (size_t c = 0 ; c < m.width(); ++c)
        {
            printf("%d ", (int)m(c, r));
        }
        printf("\n");
    }
    printf("\n");
}

bool KeypadMatrix::hasKeyPressed() const
{
    bool isPressed = false;

    for (size_t r = 0 ; (r < mKeysState.height()) && (false == isPressed) ; ++r)
    {
        for (size_t c = 0 ; (c < mKeysState.width()) && (false == isPressed); ++c)
        {
            isPressed = mKeysState(c, r);
        }
    }

    return isPressed;
}

// TODO: does pressing multiple keys on the same row causes short?
void KeypadMatrix::onPinEdgeEvent(const RP_GPIO pin, const GPIO_PIN_EDGE_EVENT event)
{
    TRACE_CALL_DEBUG_ARGS("********************** pin=%d, event=%d", SC2INT(pin), SC2INT(event));

    int x = -1;
    int y = -1;
    KeypadEvent keyEvent = KeypadEvent::UNKNOWN;
    std::vector<int> colValues;
    auto itRow = std::find(mRowPins.begin(), mRowPins.end(), pin);

    if (itRow != mRowPins.end())
    {
        matrix<bool> oldKeysState = mKeysState;

        y = itRow - mRowPins.begin();

        if (true == getGroupValues(mColsGroupID, colValues))
        {
            auto itActiveCol = std::find(colValues.begin(), colValues.end(), 1);

            if (itActiveCol != colValues.end())
            {
                x = itActiveCol - colValues.begin();
            }
        }

        TRACE_DEBUG("------- x=%d, y=%d", x, y);

        if (x >= 0)
        {
            if (false == hasKeyPressed())
            {
                TRACE_LINE();
                mKeysState.set(x, y, true);
            }
            else if (mKeysState(x, y) == true)
            {
                mKeysState.setRowValue(y, false);
            }
        }
        else
        {
            mKeysState.setRowValue(y, false);
        }

        dumpMatrix(oldKeysState);
        dumpMatrix(mKeysState);

        matrix<bool>::MatrixDiff_t diff = mKeysState.compare(oldKeysState);

        for (const auto& it : diff)
        {
            TRACE_DEBUG("DIFF: x=%d, y=%d", it.first, it.second);
            if (it.second == y)
            {
                x = it.first;

                if (true == mKeysState(it.first, it.second))
                {
                    keyEvent = KeypadEvent::KEY_PRESSED;
                }
                else
                {
                    keyEvent = KeypadEvent::KEY_RELEASED;
                }
                break;
            }
        }

        if (diff.size() > 0)
        {
            TRACE_DEBUG("x=%d, y=%d, keyEvent=%d", x, y, SC2INT(keyEvent));

            if (mOnKeyEventCallback)
            {
                std::string key;
                auto itKey = mKeys.find(std::make_pair(x, y));

                if (mKeys.end() != itKey)
                {
                    key = itKey->second;
                }

                mOnKeyEventCallback(keyEvent, x, y, key);
            }
        }

        // NOTE: need to stop edge detection so we can reset col values
        for (RP_GPIO curRowPin: mRowPins)
        {
            closePin(curRowPin);
        }

        std::vector<int> colValuesLow(mColPins.size(), 0);
        setGroupValues(mColsGroupID, colValuesLow);

        for (RP_GPIO curRowPin: mRowPins)
        {
            openPin(curRowPin, GPIO_PIN_MODE::EDGE_DETECTION, GPIO_PIN_PULL::PULL_UP);
        }
    }
}