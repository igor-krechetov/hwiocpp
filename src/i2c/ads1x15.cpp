/*
 * Copyright (C) 2022 Igor Krechetov
 * Distributed under MIT License. See file LICENSE for details (http://www.opensource.org/licenses/MIT)
 */
#include "i2c/ads1x15.hpp"
#include <stdio.h>

bool ADS1X15::initialize(const int adapterNumber, const int address)
{
    return openDevice(adapterNumber, address);
}

int16_t ADS1X15::readSingleChannel(uint8_t channel)
{
    int16_t result = 0;

    if (channel < 4)
    {
        // Start with default values
        uint16_t config = ADS1X15_REG_CONFIG_CQUE_NONE |    // Disable the comparator (default val)
                          ADS1X15_REG_CONFIG_CLAT_NONLAT |  // Non-latching (default val)
                          ADS1X15_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low     (default val)
                          ADS1X15_REG_CONFIG_CMODE_TRAD |   // Traditional comparator (default val)
                          ADS1X15_REG_CONFIG_MODE_SINGLE;   // Single-shot mode (default)

        // Set PGA/voltage range
        config |= static_cast<int>(mGain);

        // Set data rate
        config |= mDataRate;

        // Set single-ended input channel
        switch (channel) 
        {
            case 0:
                config |= ADS1X15_REG_CONFIG_MUX_SINGLE_0;
                break;
            case 1:
                config |= ADS1X15_REG_CONFIG_MUX_SINGLE_1;
                break;
            case 2:
                config |= ADS1X15_REG_CONFIG_MUX_SINGLE_2;
                break;
            case 3:
                config |= ADS1X15_REG_CONFIG_MUX_SINGLE_3;
                break;
        }

        // Set 'start single-conversion' bit
        config |= ADS1X15_REG_CONFIG_OS_SINGLE;

        // Write config register to the ADC
        writeRegister(ADS1X15_REG_POINTER_CONFIG, config);

        // TODO: add timeout
        // Wait for the conversion to complete
        while (false == conversionComplete())
        {}

        // Read the conversion results
        result = getLastConversionResults();
    }

    return result;
}

int16_t ADS1X15::readDifferential_0_1()
{
    // Start with default values
    uint16_t config = ADS1X15_REG_CONFIG_CQUE_NONE |    // Disable the comparator (default val)
                      ADS1X15_REG_CONFIG_CLAT_NONLAT |  // Non-latching (default val)
                      ADS1X15_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low     (default val)
                      ADS1X15_REG_CONFIG_CMODE_TRAD |   // Traditional comparator (default val)
                      ADS1X15_REG_CONFIG_MODE_SINGLE;   // Single-shot mode (default)

    // Set PGA/voltage range
    config |= static_cast<int>(mGain);

    // Set data rate
    config |= mDataRate;

    // Set channels
    config |= ADS1X15_REG_CONFIG_MUX_DIFF_0_1; // AIN0 = P, AIN1 = N

    // Set 'start single-conversion' bit
    config |= ADS1X15_REG_CONFIG_OS_SINGLE;

    // Write config register to the ADC
    writeRegister(ADS1X15_REG_POINTER_CONFIG, config);

    // Wait for the conversion to complete
    while (false == conversionComplete())
    {}

    // Read the conversion results
    return getLastConversionResults();
}

int16_t ADS1X15::readDifferential_2_3()
{
    // Start with default values
    uint16_t config = ADS1X15_REG_CONFIG_CQUE_NONE |    // Disable the comparator (default val)
                      ADS1X15_REG_CONFIG_CLAT_NONLAT |  // Non-latching (default val)
                      ADS1X15_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low     (default val)
                      ADS1X15_REG_CONFIG_CMODE_TRAD |   // Traditional comparator (default val)
                      ADS1X15_REG_CONFIG_MODE_SINGLE;   // Single-shot mode (default)

    // Set PGA/voltage range
    config |= static_cast<int>(mGain);

    // Set data rate
    config |= mDataRate;

    // Set channels
    config |= ADS1X15_REG_CONFIG_MUX_DIFF_2_3; // AIN2 = P, AIN3 = N

    // Set 'start single-conversion' bit
    config |= ADS1X15_REG_CONFIG_OS_SINGLE;

    // Write config register to the ADC
    writeRegister(ADS1X15_REG_POINTER_CONFIG, config);

    // Wait for the conversion to complete
    while (false == conversionComplete())
    {}

    // Read the conversion results
    return getLastConversionResults();
}

void ADS1X15::startComparator_SingleEnded(uint8_t channel, int16_t threshold)
{
    if (channel < 4)
    {
        // Start with default values
        uint16_t config = ADS1X15_REG_CONFIG_CQUE_1CONV |   // Comparator enabled and asserts on 1 match
                        ADS1X15_REG_CONFIG_CLAT_LATCH |   // Latching mode
                        ADS1X15_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low     (default val)
                        ADS1X15_REG_CONFIG_CMODE_TRAD |   // Traditional comparator (default val)
                        ADS1X15_REG_CONFIG_MODE_CONTIN |  // Continuous conversion mode
                        ADS1X15_REG_CONFIG_MODE_CONTIN;   // Continuous conversion mode

        // Set PGA/voltage range
        config |= static_cast<int>(mGain);

        // Set data rate
        config |= mDataRate;

        // Set single-ended input channel
        switch (channel)
        {
            case 0:
                config |= ADS1X15_REG_CONFIG_MUX_SINGLE_0;
                break;
            case 1:
                config |= ADS1X15_REG_CONFIG_MUX_SINGLE_1;
                break;
            case 2:
                config |= ADS1X15_REG_CONFIG_MUX_SINGLE_2;
                break;
            case 3:
                config |= ADS1X15_REG_CONFIG_MUX_SINGLE_3;
                break;
        }

        // Set the high threshold register
        // Shift 12-bit results left 4 bits for the ADS1015
        writeRegister(ADS1X15_REG_POINTER_HITHRESH, threshold << mBitShift);

        // Write config register to the ADC
        writeRegister(ADS1X15_REG_POINTER_CONFIG, config);
    }
}

int16_t ADS1X15::getLastConversionResults()
{
    // Read the conversion results
    uint16_t res = readRegister(ADS1X15_REG_POINTER_CONVERT) >> mBitShift;

    if (0 != mBitShift)
    {
        // Shift 12-bit results right 4 bits for the ADS1015,
        // making sure we keep the sign bit intact
        if (res > 0x07FF)
        {
            // negative number - extend the sign to 16th bit
            res |= 0xF000;
        }
    }

    return static_cast<int16_t>(res);
}

float ADS1X15::computeVolts(int16_t counts)
{
    // see data sheet Table 3
    float fsRange;

    switch (mGain)
    {
        case adsGain_t::GAIN_TWOTHIRDS:
            fsRange = 6.144f;
            break;
        case adsGain_t::GAIN_ONE:
            fsRange = 4.096f;
            break;
        case adsGain_t::GAIN_TWO:
            fsRange = 2.048f;
            break;
        case adsGain_t::GAIN_FOUR:
            fsRange = 1.024f;
            break;
        case adsGain_t::GAIN_EIGHT:
            fsRange = 0.512f;
            break;
        case adsGain_t::GAIN_SIXTEEN:
            fsRange = 0.256f;
            break;
        default:
            fsRange = 0.0f;
    }

    return counts * (fsRange / (32768 >> mBitShift));
}

bool ADS1X15::conversionComplete()
{
    return (readRegister(ADS1X15_REG_POINTER_CONFIG) & 0x8000) != 0;
}

void ADS1X15::writeRegister(uint8_t reg, uint16_t value)
{
    // mBuffer[0] = reg;
    // mBuffer[1] = value >> 8;// HIGH
    // mBuffer[2] = value & 0xFF;// LOW
    // writeBuffer(mBuffer, 3);
    writeData(reg, value, Endianness::BIG);
}

uint16_t ADS1X15::readRegister(uint8_t reg)
{
    return readWord(reg, Endianness::BIG);
}