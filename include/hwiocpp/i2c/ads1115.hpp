/*
 * Copyright (C) 2022 Igor Krechetov
 * Distributed under MIT License. See file LICENSE for details (http://www.opensource.org/licenses/MIT)
 */
#ifndef HWIOCPP_I2C_ADS1115_HPP
#define HWIOCPP_I2C_ADS1115_HPP

#include "ads1x15.hpp"

class ADS1115: public ADS1X15
{
public:
    ADS1115()
    {
        mBitShift = 0;
        mGain = adsGain_t::GAIN_TWOTHIRDS; // +/- 6.144V range (limited to VDD +0.3V max!)
        mDataRate = RATE_ADS1115_128SPS;
    }
    
    virtual ~ADS1115() = default;
};

#endif // HWIOCPP_I2C_ADS1115_HPP