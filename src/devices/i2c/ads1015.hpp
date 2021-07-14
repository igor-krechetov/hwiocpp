// Based on Adafruit_ADS1X15 implementation.
// Author: Igor Krechetov

#ifndef __DEVICES_I2C_ADS1015_HPP__
#define __DEVICES_I2C_ADS1015_HPP__

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

#endif // __DEVICES_I2C_ADS1015_HPP__