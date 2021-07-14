#ifndef __DEVICES_I2C_SENSORS_AHT10_HPP__
#define __DEVICES_I2C_SENSORS_AHT10_HPP__

#include "devices/i2c/DeviceI2C.hpp"

/*
 NOTE:
   Humidity:
    - Longterm exposure to conditions outside the normal range,
      especially at humidity >80 %, may result in temporary
      signal drift (drift after 60hours +3% RH ). When
      returning to normal operating conditions, the sensor
      will slowly self-recover to the calibration state
    - relative humidity range      0%..100%
    - relative humidity resolution 0.024%
    - relative humidity accuracy   ±2%

    Temperature:
    - temperature range      -40°C..+80°C
    - temperature resolution 0.01°C
    - temperature accuracy   ±0.3°C
*/

struct SensorDataAHT10
{
    double temperature = 0.0;// celsius
    int humidity = 0;// %
};

class AHT10: protected DeviceI2C
{
public:
    virtual ~AHT10();

    bool initialize(const int adapterNumber);
    SensorDataAHT10 getSensorData();

    inline DeviceI2C* getDevice();

private:
    // false - sensor idle and sleeping
    // true - sensor busy and in measurement state
    bool isBusy();
};

inline DeviceI2C* AHT10::getDevice()
{
    return this;
}

#endif // __DEVICES_I2C_SENSORS_AHT10_HPP__