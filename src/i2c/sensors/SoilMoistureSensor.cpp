/*
 * Copyright (C) 2022 Igor Krechetov
 * Distributed under MIT License. See file LICENSE for details (http://www.opensource.org/licenses/MIT)
 */
#include "i2c/sensors/SoilMoistureSensor.hpp"
#include "i2c/ads1x15.hpp"

SoilMoistureSensor::SoilMoistureSensor(ADS1X15* ads, const int adsChannel)
    : mADS(ads)
    , mAdsChannel(adsChannel)
    , mLastSensorValue(0)
    , mDryValue(22000)
    , mWaterValue(7963)
{
    // TODO: add default calibration based on sensor type
}

SoilMoistureSensor::~SoilMoistureSensor(){}

void SoilMoistureSensor::clibrate(const int dryValue, const int waterValue)
{
    mDryValue = dryValue;
    mWaterValue = waterValue;
}

double SoilMoistureSensor::getMoistureLevel(const bool forceUpdate)
{
    if (true == forceUpdate)
    {
        readSensorValue();
    }

    return GenericDevice::remap(mLastSensorValue, mDryValue, mWaterValue, 0.0, 100.0);
}

int SoilMoistureSensor::getRawSensorValue(const bool forceUpdate)
{
    if (true == forceUpdate)
    {
        readSensorValue();
    }

    return mLastSensorValue;
}

void SoilMoistureSensor::readSensorValue()
{
    // TODO: add different logic based on sensor type (if needed)
    if (nullptr != mADS)
    {
        mADS->setGain(adsGain_t::GAIN_ONE);
        // mADS->setGain(adsGain_t::GAIN_TWO);
        // mADS->setDataRate(RATE_ADS1115_8SPS);
        mLastSensorValue = mADS->readSingleChannel(mAdsChannel);
    }
}