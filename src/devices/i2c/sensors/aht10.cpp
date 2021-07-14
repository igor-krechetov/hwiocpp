#include "aht10.hpp"
#include <cstdio>

#define AHT10_ADDRESS                   0x38

#define AHT10_CMD_INIT                  0xE1 // 0b11100001
#define AHT10_CMD_TRIGGER_MEASUREMENT   0xAC // 0b10101100
#define AHT10_CMD_NORMAL                0xA8
#define AHT10_CMD_SOFT_RESET            0xBA // 0b10111010

#define AHT10_MODE_NORMAL               0x00
#define AHT10_MODE_CYCLE                0x20
#define AHT10_MODE_CMD                  0x40
#define AHT10_MODE_DEF_CALIBRATION      0x08

#define AHT10_DATA_MEASURMENT_CMD       0b00110011

#define AHT10_DELAY_MEASURMENT          100
#define AHT10_DELAY_MEASURMENT_RETRY    300
#define AHT10_DELAY_POWER_ON            100 
#define AHT10_DELAY_CMD                 350
#define AHT10_DELAY_SOFT_RESET          20 

#define MAX_READ_ATTEMPS                2

#define AHT10_STATUS_BIT_BUSY           0b10000000
#define AHT10_STATUS_BIT_CALIBRATION    0b00001000
#define AHT10_STATUS_BIT_MODE           0b01100000


AHT10::~AHT10()
{
}

bool AHT10::initialize(const int adapterNumber)
{
    bool result = false;

    if (true == openDevice(adapterNumber, AHT10_ADDRESS))
    {
        const byte mode = AHT10_MODE_DEF_CALIBRATION | AHT10_MODE_CYCLE;

        writeBuffer(AHT10_CMD_INIT, {mode, 0x00});
        wait(AHT10_DELAY_POWER_ON);

        const byte status = readByte();

        printf("CONFIG => 0x%X\n", status);

        if (status & AHT10_STATUS_BIT_CALIBRATION)
        {
            result = true;
        }
    }

    if (false == result)
    {
        closeDevice();
    }

    return result;
}

SensorDataAHT10 AHT10::getSensorData()
{
    SensorDataAHT10 result;

    if (true == writeBuffer({AHT10_CMD_TRIGGER_MEASUREMENT, AHT10_DATA_MEASURMENT_CMD, 0x00}))
    {
        int attempt = 0;

        wait(AHT10_DELAY_MEASURMENT);

        while ((attempt < MAX_READ_ATTEMPS) && (true == isBusy()))
        {
            wait(AHT10_DELAY_MEASURMENT_RETRY);
            ++attempt;
        }

        if (MAX_READ_ATTEMPS != attempt)
        {
            byte blockData[6] = {0};

            if (sizeof(blockData) == readBuffer(blockData, sizeof(blockData)))
            {
                for (int j = 0; j < sizeof(blockData); ++j)
                {
                    printf("0x%X ", blockData[j]);
                }
                printf("\n");
                unsigned int temp;
                
                temp = ((blockData[3] & 0x0F) << 16) | (blockData[4] << 8) | blockData[5];
                result.temperature = ((temp * 200) / 1048576) - 50;

                temp = ((blockData[1] << 16) | (blockData[2] << 8) | blockData[3]) >> 4;
                result.humidity = temp * 100 / 1048576;
            }
        }
        else
        {
            printf("ERROR: Failed to read data. Sensor is busy\n");
        }
    }

    return result;
}

bool AHT10::isBusy()
{
    byte status = readByte();

    return (status & AHT10_STATUS_BIT_BUSY) != 0;
}


/**********************************************************
 * GetDewPoint
 *  Gets the current dew point based on the current humidity and temperature
 *
 * @return float - The dew point in Deg C
 **********************************************************/
// float AHT10Class::GetDewPoint(void)
// {
//   float humidity = GetHumidity();
//   float temperature = GetTemperature();

//   // Calculate the intermediate value 'gamma'
//   float gamma = log(humidity / 100) + WATER_VAPOR * temperature / (BAROMETRIC_PRESSURE + temperature);
//   // Calculate dew point in Celsius
//   float dewPoint = BAROMETRIC_PRESSURE * gamma / (WATER_VAPOR - gamma);

//   return dewPoint;
// }