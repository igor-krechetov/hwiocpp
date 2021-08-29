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
#include "devices/gpio/KeypadMatrix.hpp"

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
    SoilMoistureSensor sensor(&ads, ADS_CHANNEL_A0);

    ads.setDataRate(RATE_ADS1115_8SPS);
     
    if (ads.initialize(I2C_ADAPTER_DEFAULT, ADS1X15_ADDRESS))
    {
        for (int i = 0 ; i < 2; ++i)
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
    const RP_GPIO latchPin = RP_GPIO::GPIO_21;
    const RP_GPIO clockPin = RP_GPIO::GPIO_20;
    const RP_GPIO dataPin = RP_GPIO::GPIO_16;

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

void test_combined()
{
    DeviceGPIO gpioDev;

    // 0 - sensors; 1 - relay
    const RP_GPIO modePin = RP_GPIO::GPIO_12;

    const RP_GPIO pinLED = RP_GPIO::GPIO_23;
    const RP_GPIO pinBuzzer = RP_GPIO::GPIO_18;

    // sensor pins
    const RP_GPIO pinA = RP_GPIO::GPIO_16;
    const RP_GPIO pinB = RP_GPIO::GPIO_20;
    const RP_GPIO pinC = RP_GPIO::GPIO_21;

    // relay control pins
    // 14 - ser (data)
    // 12 - storage register clock  RCLK (latch)
    // 11 - shift register clock (SRCLK) 
    const RP_GPIO dataPin = pinA;
    const RP_GPIO latchPin = pinC;
    const RP_GPIO clockPin = pinB;

    if (gpioDev.openDevice())
    {
        // turn off sensors
        // gpioDev.setPinValue(modePin, 1);

        // for (int i = 0 ; i < 6; ++i)
        // {
        //     printf("--- turn LED on\n");
        //     gpioDev.setPinValue(pinLED, 1);
        //     printf("--- turn Buzzer on\n");
        //     gpioDev.setPinValue(pinBuzzer, 1);
        //     gpioDev.wait(500);
        //     printf("--- turn LED off\n");
        //     gpioDev.setPinValue(pinLED, 0);
        //     gpioDev.setPinValue(pinBuzzer, 0);

        //     gpioDev.wait(500);
        // }

        printf("--- enable SENSORS\n");
        gpioDev.setPinValue(modePin, 0);
        test_74hc4051();

        // printf("--- enable RELAY\n");
        // gpioDev.setPinValue(modePin, 1);
        // test_sr_relay();

        printf("--- reset\n");
        gpioDev.setPinValue(modePin, 1);
        gpioDev.setPinValue(pinA, 0);
        gpioDev.setPinValue(pinB, 0);
        gpioDev.setPinValue(pinC, 0);
    }
}

void test_water_level()
{
    ADS1115 ads;

    ads.setDataRate(RATE_ADS1115_8SPS);
    ads.setGain(adsGain_t::GAIN_TWO);
     
    if (ads.initialize(I2C_ADAPTER_DEFAULT, ADS1X15_ADDRESS))
    {
        ads.printCapabilities();

        for (int i = 0 ; i < 100; ++i)
        {
            int16_t val = ads.readSingleChannel(ADS_CHANNEL_A1);
            printf("val=%d\n", (int)val);
            GenericDevice::wait(500);
        }
    }
}

void edgeCallback(const RP_GPIO pin, const GPIO_PIN_EDGE_EVENT event)
{
    printf("--- pin=%d, event=%d\n", SC2INT(pin), SC2INT(event));
}

void test_edge()
{
    DeviceGPIO gpioDev;
    // const RP_GPIO pinHigh = RP_GPIO::GPIO_19;
    const RP_GPIO pin = RP_GPIO::GPIO_19;
    const RP_GPIO pinLow = RP_GPIO::GPIO_13;

    if (gpioDev.openDevice())
    {
        gpioDev.registerEdgeEventsCallback(edgeCallback);

        gpioDev.setPinValue(pinLow, 0);
        gpioDev.openPin(pin, GPIO_PIN_MODE::EDGE_DETECTION, GPIO_PIN_PULL::PULL_UP);
        // gpioDev.openPin(pinLow, GPIO_PIN_MODE::EDGE_DETECTION, GPIO_PIN_PULL::PULL_UP);

        gpioDev.wait(100000);
    }
}

void onKeypadEvent(const KeypadEvent event, const int x, const int y, const std::string& key)
{
    printf("========= KEY: <%s> (%d, %d, event=%s)\n",
           key.c_str(), x, y, (event == KeypadEvent::KEY_PRESSED ? "PRESSED" : "RELEASED"));
}

void test_keypad_4x4()
{
    KeypadMatrix pad;

    pad.initialize({RP_GPIO::GPIO_06, RP_GPIO::GPIO_13, RP_GPIO::GPIO_19, RP_GPIO::GPIO_26},
                    {RP_GPIO::GPIO_17, RP_GPIO::GPIO_27, RP_GPIO::GPIO_22, RP_GPIO::GPIO_25},
                    onKeypadEvent);
    DeviceGPIO::wait(100000);
}

void test_keypad_5x4()
{
    KeypadMatrix pad;
    KeypadKeyMap_t keys = {{{3,0}, "F1"}, {{2,0}, "F2"}, {{1,0}, "#"}, {{0,0}, "*"},
                           {{3,1}, "1"}, {{2,1}, "2"}, {{1,1}, "3"}, {{0,1}, "UP"},
                           {{3,2}, "4"}, {{2,2}, "5"}, {{1,2}, "6"}, {{0,2}, "DOWN"},
                           {{3,3}, "7"}, {{2,3}, "8"}, {{1,3}, "9"}, {{0,3}, "ESC"},
                           {{3,4}, "LEFT"}, {{2,4}, "0"}, {{1,4}, "RIGHT"}, {{0,4}, "ENT"}};
    
    // {RP_GPIO::GPIO_05, RP_GPIO::GPIO_06, RP_GPIO::GPIO_13, RP_GPIO::GPIO_19, RP_GPIO::GPIO_26}
    // {RP_GPIO::GPIO_26, RP_GPIO::GPIO_19, RP_GPIO::GPIO_13, RP_GPIO::GPIO_06, RP_GPIO::GPIO_05}

    // {RP_GPIO::GPIO_22, RP_GPIO::GPIO_27, RP_GPIO::GPIO_17, RP_GPIO::GPIO_04}
    // {RP_GPIO::GPIO_04, RP_GPIO::GPIO_17, RP_GPIO::GPIO_27, RP_GPIO::GPIO_22}
    pad.initialize({RP_GPIO::GPIO_26, RP_GPIO::GPIO_19, RP_GPIO::GPIO_13, RP_GPIO::GPIO_06, RP_GPIO::GPIO_05},
                   {RP_GPIO::GPIO_22, RP_GPIO::GPIO_27, RP_GPIO::GPIO_17, RP_GPIO::GPIO_04},
                    onKeypadEvent);
    pad.setKeymapping(keys);
    DeviceGPIO::wait(100000);
}

void test_gpio_pin()
{
    DeviceGPIO gpioDev;
    const RP_GPIO pin = RP_GPIO::GPIO_19;

    if (gpioDev.openDevice())
    {
        int val = 0;
        gpioDev.setPinValue(pin, 1);
        
        if (true == gpioDev.getPinValue(pin, val))
        {
            printf("value=%d\n", val);
        }
        else
        {
            printf("error\n");
        }
    }
}

int test_waterLevelSwitch()
{
    DeviceGPIO dev;

    if (dev.openDevice())
    {
        int oldValue = 1;
        dev.openPin(RP_GPIO::GPIO_08, GPIO_PIN_MODE::INPUT, GPIO_PIN_PULL::PULL_UP);

        for(int i = 0 ; i < 400; ++i)
        {
            int val = 0;
            
            if (true == dev.getPinValue(RP_GPIO::GPIO_08, val))
            {
                printf("val=%d\n", val);
                if (val == 0 && oldValue != val)
                {
                    printf("CLOSED!\n");
                }
            }

            oldValue = val;
            dev.wait(250);
        }
    }
}

int main(const int argc, const char**argv)
{
    __TRACE_INIT__();

    // if (argc == 3)
    // {
    //     GPIO_PIN_PULL mode;

    //     switch(atoi(argv[2]))
    //     {
    //         case 0:
    //             mode = GPIO_PIN_PULL::DISABLE;
    //             break;
    //         case 1:
    //             mode = GPIO_PIN_PULL::PULL_UP;
    //             break;
    //         case 2:
    //             mode = GPIO_PIN_PULL::PULL_DOWN;
    //             break;
    //     }

    //     DeviceGPIO::gpio_set_pull(atoi(argv[1]), mode);
    //     printf("done\n");
    //     // 10 -> 19
    //     // 11 -> 23
    //     // 12 -> 32
    //     // 13 -> 33
    //     // 14 -> 8
    //     // 15 -> 10
    //     // 16 -> 36
    //     // 18 -> 12
    //     // 19 -> 35
    // }

    // return 0;

    // i2c_test();
    // gpio_test();
    // ads1115_test();
    // soil_test();
    // test_shift_register();

    // test_74hc4051();
    // test_sr_relay();
            // test_combined();
    // test_water_level();
    // test_edge();
    test_keypad_5x4();
    // test_gpio_pin();
    // test_waterLevelSwitch();


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