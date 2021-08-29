#ifndef __DEVICES_I2C_SENSORS_WATERLEVELSENSOR_HPP__
#define __DEVICES_I2C_SENSORS_WATERLEVELSENSOR_HPP__

class ADS1X15;

// NOTE: 

class WaterSensor
{
public:
    WaterLevelSensor(ADS1X15* ads, const int adsChannel);
    ~WaterLevelSensor();

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

#endif // __DEVICES_I2C_SENSORS_WATERLEVELSENSOR_HPP__