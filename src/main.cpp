#include <chrono>
#include <thread>
#include <string>
#include <hsmcpp/HsmEventDispatcherSTD.hpp>
#include "gen/SwitchHsmBase.hpp"
// #include <pigpio.h>
#include <lgpio/lgpio.h>
#include "hsmcpp/logging.hpp"
#include "devices/i2c/sensors/aht10.hpp"
#include "devices/i2c/sensors/SoilMoistureSensor.hpp"
#include "devices/gpio/DeviceGPIO.hpp"
#include "devices/gpio/Relay.hpp"
#include "devices/gpio/74hc4051.hpp"
#include "devices/i2c/ads1115.hpp"

__TRACE_PREINIT__();

using namespace std::chrono_literals;

// NOTE: http://abyz.me.uk/lg/lgpio.html

class SwitchHsm: public SwitchHsmBase
{
public:
    virtual ~SwitchHsm(){}

// HSM state changed callbacks
protected:
    void onOff(const VariantList_t& args) override
    {
        printf("Off\n");
        std::this_thread::sleep_for(1000ms);
        transition(SwitchHsmEvents::SWITCH);
    }

    void onOn(const VariantList_t& args) override
    {
        printf("On\n");
        std::this_thread::sleep_for(1000ms);
        transition(SwitchHsmEvents::SWITCH);
    }
};


void i2c_test()
{
    AHT10 sensor;

    if (true == sensor.initialize(1))
    {
        sensor.getDevice()->printCapabilities();

        for (int i = 0 ; i < 2 ; ++i)
        {
            SensorDataAHT10 data = sensor.getSensorData();

            printf("Temperature: %.1f C\n", data.temperature);
            printf("Humidity: %d %%\n\n", data.humidity);
            GenericDevice::wait(1000);
        }
    }
}

void ads1115_test()
{
    ADS1115 ads;

    ads.setDataRate(RATE_ADS1115_8SPS);
    ads.setGain(adsGain_t::GAIN_TWO);
     
    if (ads.initialize(I2C_ADAPTER_DEFAULT, ADS1X15_ADDRESS))
    {
        ads.printCapabilities();

        for (int i = 0 ; i < 5; ++i)
        {
            int16_t val = ads.readSingleChannel(ADS_CHANNEL_A2);
            printf("val=%d\n", (int)val);
            GenericDevice::wait(1000);
        }
    }
}

void soil_test()
{
    ADS1115 ads;
    SoilMoistureSensor sensor(&ads, ADS_CHANNEL_A3);

    ads.setDataRate(RATE_ADS1115_8SPS);
     
    if (ads.initialize(I2C_ADAPTER_DEFAULT, ADS1X15_ADDRESS))
    {
        for (int i = 0 ; i < 3; ++i)
        {
            printf("moisture=%.2f %, sensor value=%d\n", sensor.getMoistureLevel(), sensor.getRawSensorValue(false));
            GenericDevice::wait(700);
        }
    }
}

void gpio_test()
{
    DeviceGPIO gpio2;

    if (gpio2.openDevice())
    {
        gpio2.setPinValue(RP_GPIO::GPIO_25, 0);
        gpio2.wait(200);

        gpio2.setPinValue(RP_GPIO::GPIO_25, 1);
        gpio2.wait(5000);

        gpio2.setPinValue(RP_GPIO::GPIO_25, 0);
        gpio2.wait(200);
    }
}

void test_shift_register()
{
    DeviceGPIO gpio2;
    // 16 - ser
    // 20 - storage register clock  RCLK (latch)
    // 21 - shift register clock (SRCLK) 
    const RP_GPIO latchPin = RP_GPIO::GPIO_24;
    const RP_GPIO clockPin = RP_GPIO::GPIO_25;
    const RP_GPIO dataPin = RP_GPIO::GPIO_23;
    // const RP_GPIO latchPin = RP_GPIO::GPIO_20;
    // const RP_GPIO clockPin = RP_GPIO::GPIO_21;
    // const RP_GPIO dataPin = RP_GPIO::GPIO_16;

    if (gpio2.openDevice())
    {
        gpio2.setPinValue(clockPin, 0);
        gpio2.setPinValue(latchPin, 0);
        gpio2.setPinValue(dataPin, 0);
        usleep(1);

        for (int i = 1 ; i < 16; i++)
        {
            gpio2.shiftWrite(i, dataPin, clockPin, latchPin);
            gpio2.wait(250);
        }

        gpio2.shiftWrite(0, dataPin, clockPin, latchPin);

        gpio2.setPinValue(clockPin, 0);
        gpio2.setPinValue(latchPin, 0);
        gpio2.setPinValue(dataPin, 0);
    }

    printf("test_shift_register - DONE\n");
}

void test_74hc4051()
{
    printf("-> test_74hc4051\n");
    Dev74HC4051 dev;
    const RP_GPIO pinA = RP_GPIO::GPIO_16;
    const RP_GPIO pinB = RP_GPIO::GPIO_20;
    const RP_GPIO pinC = RP_GPIO::GPIO_21;

    // C -> A
    // 1 -> X4 (C1 B0 A0)
    // 3 -> X5 (C1 B0 A1)

    if (dev.initialize(pinA, pinB, pinC, 0))
    {
        for (int i = 0 ; i < 4; i++)
        {
            printf("test_74hc4051: X%d enabled\n", i);
            dev.selectChannel(i);
            soil_test();
        }
    }
}

void test_relay()
{
    Relay dev1;
    DeviceGPIO gpio2;

    if (dev1.initialize({RP_GPIO::GPIO_25}, {RelayNormalState::NORMALLY_OPEN}))
    {
        if (gpio2.openDevice())
        {
            gpio2.setPinValue(RP_GPIO::GPIO_20, 0);
            gpio2.wait(200);

            dev1.closeRelay(0);
            gpio2.setPinValue(RP_GPIO::GPIO_20, 1);
            gpio2.wait(5000);

            dev1.openRelay(0);
            gpio2.setPinValue(RP_GPIO::GPIO_20, 0);
            gpio2.wait(200);
        }
    }
}

void test_sr_relay()
{
    printf("-> test_sr_relay\n");
    DeviceGPIO gpio2;
    // 16 - ser
    // 20 - storage register clock  RCLK (latch)
    // 21 - shift register clock (SRCLK) 
    const RP_GPIO latchPin = RP_GPIO::GPIO_24;
    const RP_GPIO clockPin = RP_GPIO::GPIO_25;
    const RP_GPIO dataPin = RP_GPIO::GPIO_23;

    if (gpio2.openDevice())
    {
        gpio2.setPinValue(clockPin, 0);
        gpio2.setPinValue(latchPin, 0);
        gpio2.setPinValue(dataPin, 0);
        usleep(1);

        // 1111 ^ 0001
        // 1110
        // 1101

        for (byte i = 1 ; i < 16; i++)
        {
            gpio2.shiftWrite(0xFF ^ i, dataPin, clockPin, latchPin);
            gpio2.wait(700);
        }

        gpio2.shiftWrite(0xFF, dataPin, clockPin, latchPin);

        gpio2.setPinValue(clockPin, 0);
        gpio2.setPinValue(latchPin, 0);
        gpio2.setPinValue(dataPin, 0);
    }

    printf("test_shift_register - DONE\n");
}

int main(const int argc, const char**argv)
{
    __TRACE_INIT__();

    // i2c_test();
    // gpio_test();
    // ads1115_test();
    // soil_test();
    // test_shift_register();
    test_74hc4051();
    test_sr_relay();


    // std::shared_ptr<HsmEventDispatcherSTD> dispatcher = std::make_shared<HsmEventDispatcherSTD>();
    // SwitchHsm hsm;

    // hsm.initialize(dispatcher);
    // hsm.transition(SwitchHsmEvents::SWITCH);

    // dispatcher->join();

    // gpioInitialise
    // gpioSetMode
    // gpioGetMode
    // gpioSetPullUpDown
    // gpioRead
    // gpioWrite
    // gpioTrigger
    // gpioReadBank1
    // gpioReadBank2
    // gpioClearBank1
    // gpioClearBank2
    // gpioSetBank1
    // gpioSetBank2
    // gpioHardwareRevision

    // NOTE: needs root
    // int status = gpioInitialise();
    // printf("status=%d\n", status);
    // gpioTerminate();

    // gpioSetMode(1, PI_INPUT);
    // gpioSetPullUpDown(1, PI_PUD_UP);

    // int h_i2c = lgI2cOpen(0, 0x48, int i2cFlags)
    // https://www.programmersought.com/article/90214885176/
    // https://github.com/WiringPi/WiringPi/tree/master/wiringPi

    // int h;
    // int old_v, v;
    // int lFlags = 0; /* default line flags */

    // /* get a handle to the GPIO */
    // h = lgGpiochipOpen(0);

    // if (h >= 0)
    // {
    //     /* claim a GPIO for INPUT */
    //     // lgGpioClaimInput(h, lFlags, 25);

    //     // /* claim some GPIO for OUTPUT */
    //     lgGpioClaimOutput(h, lFlags, 25, 1); /* initial level 0 */
    //     lguSleep(0.1);
    //     lgGpioWrite(h, 25, 0);
    //     // usleep(3000000);
    //     lguSleep(3.0);
    //     lgGpioWrite(h, 25, 1);

    //     lgGpioFree(h, 25);
    //     lgGpiochipClose(h);
    // }
    // else
    // {
    //     printf("FAILED\n");
    // }

    

    return 0;
}