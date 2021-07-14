// Based on Adafruit_ADS1X15 implementation.
// Author: Igor Krechetov

#ifndef __DEVICES_I2C_ADS1115_HPP__
#define __DEVICES_I2C_ADS1115_HPP__

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

#endif // __DEVICES_I2C_ADS1115_HPP__