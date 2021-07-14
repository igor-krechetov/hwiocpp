#ifndef __DEVICES_GENERICDEVICE_HPP__
#define __DEVICES_GENERICDEVICE_HPP__

#include <stdint.h>
#include <vector>

#define INVALID_FD              (-1)

#define GET_BYTE0(_val)         ((_val) & 0xFF)
#define GET_BYTE1(_val)         (((_val) >> 8) & 0xFF)
#define GET_BYTE2(_val)         (((_val) >> 16) & 0xFF)
#define GET_BYTE3(_val)         (((_val) >> 24) & 0xFF)

typedef unsigned char  byte;

enum class Endianness
{
    BIG,
    LITTLE,
    NATIVE
};

class GenericDevice
{
public:
    GenericDevice();
    virtual ~GenericDevice() = default;

    // virtual bool openDevice() = 0;
    virtual void closeDevice() = 0;
    virtual bool isDeviceOpen() = 0;

    static void wait(const unsigned int milliseconds);
    static double remap(double value, double oldMin, double oldMax, double newMin, double newMax);

    inline Endianness getNativeBytesOrder() const;
    uint16_t normalizeBytes(const uint16_t value, const Endianness expectedOrder);
    uint32_t normalizeBytes(const uint32_t value, const Endianness expectedOrder);
    // returns pointer to a buffer with normalized bytes
    // NOTE: do not free this pointer!
    // NOTE: this function is NOT tread-safe!
    const byte* normalizeBytes(const byte* buffer, const size_t bytesCount, const Endianness expectedOrder);

    // virtual int read();
    // virtual int write();

private:
    // int mFD = INVALID_FD;
    Endianness mNativeBytesOrder;
    std::vector<byte> mBuffer;// used for bytes order normalization
};

// inline bool DeviceI2C::isDeviceOpen()
// {
//     return mFD != INVALID_FD;
// }

inline Endianness GenericDevice::getNativeBytesOrder() const
{
    return mNativeBytesOrder;
}

#endif // __DEVICES_GENERICDEVICE_HPP__