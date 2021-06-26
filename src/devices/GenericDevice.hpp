#ifndef __DEVICES_GENERICDEVICE_HPP__
#define __DEVICES_GENERICDEVICE_HPP__

#define INVALID_FD              (-1)

typedef unsigned char  byte;

class GenericDevice
{
public:
    virtual ~GenericDevice() = default;

    // virtual bool openDevice() = 0;
    virtual void closeDevice() = 0;
    virtual bool isOpen() = 0;

    void wait(const unsigned int milliseconds);

    // virtual int read();
    // virtual int write();

private:
    // int mFD = INVALID_FD;
};

// inline bool DeviceI2C::isOpen()
// {
//     return mFD != INVALID_FD;
// }

#endif // __DEVICES_GENERICDEVICE_HPP__