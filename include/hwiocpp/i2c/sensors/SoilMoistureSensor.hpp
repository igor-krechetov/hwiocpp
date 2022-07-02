/*
 * Copyright (C) 2022 Igor Krechetov
 * Distributed under MIT License. See file LICENSE for details (http://www.opensource.org/licenses/MIT)
 */
#ifndef HWIOCPP_I2C_SENSORS_SOILMOISTURESENSOR_HPP
#define HWIOCPP_I2C_SENSORS_SOILMOISTURESENSOR_HPP

class ADS1X15;

class SoilMoistureSensor
{
public:
    SoilMoistureSensor(ADS1X15* ads, const int adsChannel);
    ~SoilMoistureSensor();

    // dryValue - sensor value when moisture level is at 0% (ideally measure in dry soil or atkeast in dry air)
    // waterValue - sensor value when moisture level is near maximum (wet soil or in water)
    void clibrate(const int dryValue, const int waterValue);

    // returns soil moisture level as a persentage (0% ~ 100%)
    double getMoistureLevel(const bool forceUpdate = true);
    
    int getRawSensorValue(const bool forceUpdate = true);

private:
    void readSensorValue();

private:
    ADS1X15* mADS;
    int mAdsChannel;
    int mLastSensorValue;
    int mDryValue;
    int mWaterValue;
};

#endif // HWIOCPP_I2C_SENSORS_SOILMOISTURESENSOR_HPP