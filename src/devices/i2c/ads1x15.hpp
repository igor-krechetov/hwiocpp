/**************************************************************************/
/*!
    @file     Adafruit_ADS1X15.h
    This is a library for the Adafruit ADS1X15 ADC breakout boards.
    Adafruit invests time and resources providing this open source code,
    please support Adafruit and open-source hardware by purchasing
    products from Adafruit!
    Written by Kevin "KTOWN" Townsend for Adafruit Industries.
    BSD license, all text here must be included in any redistribution
*/
/**************************************************************************/
// Based on Adafruit_ADS1X15 implementation.
// Author: Igor Krechetov

#ifndef __DEVICES_I2C_ADS1X15_HPP__
#define __DEVICES_I2C_ADS1X15_HPP__

#include "DeviceI2C.hpp"
#include "ads1x15_defines.hpp"

class ADS1X15: public DeviceI2C
{
public:
    // @brief Sets up the HW (reads coefficients values, etc.)
    // @param i2c_addr I2C address of device
    // @param wire I2C bus
    // @return true if successful, otherwise false
    bool initialize(const int adapterNumber, const int address);

    // @brief Gets a single-ended ADC reading from the specified channel (0 ~ 3)
    // @param channel 
    // @return the ADC reading
    int16_t readSingleChannel(uint8_t channel);

    // @brief  Reads the conversion results, measuring the voltage
    //         difference between the P (AIN0) and N (AIN1) input.
    //         Generates a signed value since the difference can be 
    //         either positive or negative.
    // @return the ADC reading
    int16_t readDifferential_0_1();
    // @brief Reads the conversion results, measuring the voltage
    //        difference between the P (AIN2) and N (AIN3) input.
    //        Generates a signed value since the difference can be
    //        either positive or negative.
    // @return the ADC reading
    int16_t readDifferential_2_3();

    // @brief Sets up the comparator to operate in basic mode, causing the
    //        ALERT/RDY pin to assert (go from high to low) when the ADC
    //        value exceeds the specified threshold.
    //        This will also set the ADC in continuous conversion mode.
    // @param channel ADC channel to use
    // @param threshold comparator threshold
    void startComparator_SingleEnded(uint8_t channel, int16_t threshold);

    // @brief In order to clear the comparator, we need to read the
    //        conversion results.    This function reads the last conversion
    //        results without changing the config value.
    // @return the last ADC reading
    int16_t getLastConversionResults();

    // @brief Returns true if conversion is complete, false otherwise.
    // @param counts the ADC reading in raw counts
    // @return the ADC reading in volts
    float computeVolts(int16_t counts);

    // @brief Sets the gain and input voltage range
    // @param gain gain setting to use
    inline void setGain(adsGain_t gain);

    // @brief Gets a gain and input voltage range
    // @return the gain setting
    inline adsGain_t getGain() const;

    // @brief Sets the data rate
    // @param rate the data rate to use
    inline void setDataRate(uint16_t rate);

    // @brief Gets the current data rate
    // @return the data rate
    inline uint16_t getDataRate() const;

private:
    // @brief Returns true if conversion is complete, false otherwise.
    bool conversionComplete();
    
    // @brief Writes 16-bits to the specified destination register
    // @param reg register address to write to
    // @param value value to write to register
    void writeRegister(uint8_t reg, uint16_t value);

    // @brief Read 16-bits from the specified destination register
    // @param reg register address to read from
    // @return 16 bit register value read
    uint16_t readRegister(uint8_t reg);

protected:
    uint8_t mBitShift;
    adsGain_t mGain;
    uint16_t mDataRate;
    uint8_t mBuffer[3];
};

inline void ADS1X15::setGain(adsGain_t gain)
{
    mGain = gain;
}

inline adsGain_t ADS1X15::getGain() const
{
    return mGain;
}

inline void ADS1X15::setDataRate(uint16_t rate)
{
    mDataRate = rate;
}

inline uint16_t ADS1X15::getDataRate() const
{
    return mDataRate;
}

#endif // __DEVICES_I2C_ADS1X15_HPP__