#include <chrono>
#include <thread>
#include <string>
#include <hsmcpp/HsmEventDispatcherSTD.hpp>
#include "gen/SwitchHsmBase.hpp"
// #include <pigpio.h>
#include <lgpio/lgpio.h>

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

#include "devices/i2c/aht10.hpp"

void i2c_test()
{
    AHT10 sensor;

    if (true == sensor.open(1))
    {
        sensor.printCapabilities();

        for (int i = 0 ; i < 5 ; ++i)
        {
            SensorDataAHT10 data = sensor.getSensorData();

            printf("Temperature: %.1f C\n", data.temperature);
            printf("Humidity: %d %%\n\n", data.humidity);
            sensor.wait(1000);
        }
    }
}

int main(const int argc, const char**argv)
{
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

    i2c_test();
    // i2c_test2();

    return 0;
}