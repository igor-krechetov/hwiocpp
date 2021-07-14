#ifndef __DEVICES_I2C_DEVICEI2C_HPP__
#define __DEVICES_I2C_DEVICEI2C_HPP__

#include "devices/GenericDevice.hpp"
#include <linux/i2c.h>
#include <vector>

// doc: https://www.kernel.org/doc/Documentation/i2c/
// DOC: https://www.kernel.org/doc/html/latest/driver-api/i2c.html
// doc: https://raspberry-projects.com/pi/programming-in-c/i2c/using-the-i2c-interface
// Requires: sudo apt install libi2c-dev

#define I2C_ADAPTER_DEFAULT             (1)

class DeviceI2C: public GenericDevice
{
public:
    DeviceI2C() = default;
    virtual ~DeviceI2C();

    bool openDevice(const int adapterNumber, const int address, const int capabilities = 0);
    void closeDevice() override;
    bool isDeviceOpen() override;

    byte readByte();
    byte readByte(const byte cmd);
    uint16_t readWord(const byte cmd, const Endianness bytesOrder = Endianness::NATIVE);
    int readBuffer(byte* outBuffer, const size_t bytesCount, const Endianness bytesOrder = Endianness::NATIVE);

    bool writeData(const uint8_t data);
    bool writeData(const uint16_t data, const Endianness bytesOrder = Endianness::NATIVE);
    bool writeData(const uint32_t data, const Endianness bytesOrder = Endianness::NATIVE);
    bool writeData(const byte cmd, const uint8_t data);
    bool writeData(const byte cmd, const uint16_t data, const Endianness bytesOrder = Endianness::NATIVE);
    bool writeData(const byte cmd, const uint32_t data, const Endianness bytesOrder = Endianness::NATIVE);

    bool writeBuffer(const byte* buffer, const size_t bytesCount, const Endianness bytesOrder = Endianness::NATIVE);
    bool writeBuffer(const std::vector<byte>& buffer, const Endianness bytesOrder = Endianness::NATIVE);

    bool writeBuffer(const byte cmd, const byte* buffer, const size_t bytesCount, const Endianness bytesOrder = Endianness::NATIVE);
    bool writeBuffer(const byte cmd, const std::vector<byte>& buffer, const Endianness bytesOrder = Endianness::NATIVE);

    void printCapabilities();

private:
    int mFD = INVALID_FD;
    int mCapabilites = 0;
};

inline bool DeviceI2C::isDeviceOpen()
{
    return mFD != INVALID_FD;
}

#endif // __DEVICES_I2C_DEVICEI2C_HPP__