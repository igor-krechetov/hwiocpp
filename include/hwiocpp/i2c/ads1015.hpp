/*
 * Copyright (C) 2022 Igor Krechetov
 * Distributed under MIT License. See file LICENSE for details (http://www.opensource.org/licenses/MIT)
 */
#ifndef HWIOCPP_I2C_ADS1015_HPP
#define HWIOCPP_I2C_ADS1015_HPP

#include "ads1x15.hpp"

class ADS1015: public ADS1X15
{
public:
    ADS1015()
    {
        mBitShift = 4;
        mGain = adsGain_t::GAIN_TWOTHIRDS; // +/- 6.144V range (limited to VDD +0.3V max!)
        mDataRate = RATE_ADS1015_1600SPS;
    }

    virtual ~ADS1015() = default;
};

#endif // HWIOCPP_I2C_ADS1015_HPP