#ifndef __DEVICES_GPIO_74HC4051_HPP__
#define __DEVICES_GPIO_74HC4051_HPP__

#include "DeviceGPIO.hpp"
#include <vector>

#define DEV_74HC4051_MAX_CHANNELS        (7)

// NOTE: Tested with HD74HC4051
class Dev74HC4051: protected DeviceGPIO
{
public:
    virtual ~Dev74HC4051() = default;

    bool initialize(const RP_GPIO pinA,
                    const RP_GPIO pinB,
                    const RP_GPIO pinC,
                    const unsigned int initialChannel,
                    const RP_GPIO pinInhibitor = RP_GPIO::UNKNOWN);

    // Select which output channel to emable (0 ~ 7)
    bool selectChannel(const unsigned int channel);
    inline int getCurrentChannel() const;

    bool disableOutput(const bool disable);

private:
    GpioPinsGroupID_t mPinsGroup = INVALID_GPIO_GROUP_ID;
    RP_GPIO mPinInhibitor = RP_GPIO::UNKNOWN;
    int mCurrentChannel = 0;
};

inline int Dev74HC4051::getCurrentChannel() const
{
    return mCurrentChannel;
}


#endif // __DEVICES_GPIO_74HC4051_HPP__