#include "DeviceI2C.hpp"
#include <string>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
extern "C"
{
  #include <i2c/smbus.h>
}

DeviceI2C::~DeviceI2C()
{
    closeDevice();
}

bool DeviceI2C::openDevice(const int adapterNumber, const int address, const int capabilities)
{
    bool result = false;
    char devPath[20] = {0};

    snprintf(devPath, sizeof(devPath) - 1, "/dev/i2c-%d", adapterNumber);
    mFD = open(devPath, O_RDWR);

    if (mFD > 0)
    {
        if (ioctl(mFD, I2C_SLAVE, address) >= 0)
        {
            if (ioctl(mFD, I2C_FUNCS, &mCapabilites) >= 0)
            {
                if (0 != capabilities)
                {
                    result = (mCapabilites & capabilities);
                }
                else
                {
                    result = true;
                }
            }
        }
    }
    else
    {
        mFD = INVALID_FD;
    }

    if ((false == result) && (INVALID_FD != mFD))
    {
        close(mFD);
        mFD = INVALID_FD;
        mCapabilites = 0;
    }

    return result;
}

void DeviceI2C::closeDevice()
{
    if (true == isDeviceOpen())
    {
        close(mFD);
        mFD = INVALID_FD;
        mCapabilites = 0;
    }
}

byte DeviceI2C::readByte()
{
    byte result = 0;

    if (true == isDeviceOpen())
    {
        result = i2c_smbus_read_byte(mFD);
    }

    return result;
}

byte DeviceI2C::readByte(const byte cmd)
{
    byte result = 0;

    if (true == isDeviceOpen())
    {
        result = i2c_smbus_read_byte_data(mFD, cmd);
    }

    return result;
}

uint16_t DeviceI2C::readWord(const byte cmd, const Endianness bytesOrder)
{
    uint16_t result = 0;

    if (true == isDeviceOpen())
    {
        result = i2c_smbus_read_word_data(mFD, cmd);
        result = normalizeBytes(result, bytesOrder);
    }

    return result;
}

int DeviceI2C::readBuffer(byte* outBuffer, const size_t bytesCount, const Endianness bytesOrder)
{
    // printf("READ: %lu bytes\n", bytesCount);
    int actualBytes = 0;

    if (true == isDeviceOpen())
    {
        actualBytes = read(mFD, outBuffer, bytesCount);

        const byte* normalizedBuffer = normalizeBytes(outBuffer, actualBytes, bytesOrder);

        if (outBuffer != normalizedBuffer)
        {
            memcpy(outBuffer, normalizedBuffer, bytesCount);
        }
    }

    return actualBytes;
}

bool DeviceI2C::writeData(const uint8_t data)
{
    return writeBuffer(reinterpret_cast<const byte*>(&data), sizeof(data), Endianness::NATIVE);
}

bool DeviceI2C::writeData(const uint16_t data, const Endianness bytesOrder)
{
    return writeBuffer(reinterpret_cast<const byte*>(&data), sizeof(data), bytesOrder);
}

bool DeviceI2C::writeData(const uint32_t data, const Endianness bytesOrder)
{
    return writeBuffer(reinterpret_cast<const byte*>(&data), sizeof(data), bytesOrder);
}

bool DeviceI2C::writeData(const byte cmd, const uint8_t data)
{
    return writeBuffer(cmd, reinterpret_cast<const byte*>(&data), sizeof(data), Endianness::NATIVE);
}

bool DeviceI2C::writeData(const byte cmd, const uint16_t data, const Endianness bytesOrder)
{
    // printf("DeviceI2C::writeData: cmd=%d, data=%d (%X)\n", (int)cmd, (int)data, (int)data);
    return writeBuffer(cmd, reinterpret_cast<const byte*>(&data), sizeof(data), bytesOrder);
}

bool DeviceI2C::writeData(const byte cmd, const uint32_t data, const Endianness bytesOrder)
{
    return writeBuffer(cmd, reinterpret_cast<const byte*>(&data), sizeof(data), bytesOrder);
}

bool DeviceI2C::writeBuffer(const byte* buffer, const size_t bytesCount, const Endianness bytesOrder)
{
    // printf("DeviceI2C::writeBuffer: %lu bytes\n", bytesCount);
    bool result = false;

    if (true == isDeviceOpen())
    {
        write(mFD, normalizeBytes(buffer, bytesCount, bytesOrder), bytesCount);
        result = true;
    }

    return result;
}

bool DeviceI2C::writeBuffer(const std::vector<byte>& buffer, const Endianness bytesOrder)
{
    return writeBuffer(buffer.data(), buffer.size());
}

bool DeviceI2C::writeBuffer(const byte cmd, const byte* buffer, const size_t bytesCount, const Endianness bytesOrder)
{
    // printf("WRITE: %lu bytes\n", bytesCount);
    bool result = false;

    if (true == isDeviceOpen())
    {
        // NOTE: writes: S Addr Wr [A] Comm [A] Data [A] Data [A] ... [A] Data [A] P
        //  see: https://www.kernel.org/doc/html/v5.4/i2c/smbus-protocol.html#i2c-block-write-i2c-smbus-write-i2c-block-data
        result = (0 == i2c_smbus_write_i2c_block_data(mFD, cmd, bytesCount, normalizeBytes(buffer, bytesCount, bytesOrder)));
    }

    return result;
}

bool DeviceI2C::writeBuffer(const byte cmd, const std::vector<byte>& buffer, const Endianness bytesOrder)
{
    return writeBuffer(cmd, buffer.data(), buffer.size());
}

void DeviceI2C::printCapabilities()
{
    if (true == isDeviceOpen())
    {
        std::string i2cFuncIdName[] = {"I2C_FUNC_I2C",
                                       "I2C_FUNC_10BIT_ADDR",
                                       "I2C_FUNC_PROTOCOL_MANGLING",
                                       "I2C_FUNC_NOSTART",
                                       "I2C_FUNC_SMBUS_QUICK",
                                       "I2C_FUNC_SMBUS_READ_BYTE",
                                       "I2C_FUNC_SMBUS_WRITE_BYTE",
                                       "I2C_FUNC_SMBUS_READ_BYTE_DATA",
                                       "I2C_FUNC_SMBUS_WRITE_BYTE_DATA",
                                       "I2C_FUNC_SMBUS_READ_WORD_DATA",
                                       "I2C_FUNC_SMBUS_WRITE_WORD_DATA",
                                       "I2C_FUNC_SMBUS_PROC_CALL",
                                       "I2C_FUNC_SMBUS_READ_BLOCK_DATA",
                                       "I2C_FUNC_SMBUS_WRITE_BLOCK_DATA",
                                       "I2C_FUNC_SMBUS_READ_I2C_BLOCK",
                                       "I2C_FUNC_SMBUS_WRITE_I2C_BLOCK",
                                       "I2C_FUNC_SMBUS_BYTE",
                                       "I2C_FUNC_SMBUS_BYTE_DATA",
                                       "I2C_FUNC_SMBUS_WORD_DATA",
                                       "I2C_FUNC_SMBUS_BLOCK_DATA",
                                       "I2C_FUNC_SMBUS_I2C_BLOCK",
                                       "I2C_FUNC_SMBUS_EMUL"};
        int i2cFuncId[] = { I2C_FUNC_I2C,
                            I2C_FUNC_10BIT_ADDR,
                            I2C_FUNC_PROTOCOL_MANGLING,
                            I2C_FUNC_NOSTART,
                            I2C_FUNC_SMBUS_QUICK,
                            I2C_FUNC_SMBUS_READ_BYTE,
                            I2C_FUNC_SMBUS_WRITE_BYTE,
                            I2C_FUNC_SMBUS_READ_BYTE_DATA,
                            I2C_FUNC_SMBUS_WRITE_BYTE_DATA,
                            I2C_FUNC_SMBUS_READ_WORD_DATA,
                            I2C_FUNC_SMBUS_WRITE_WORD_DATA,
                            I2C_FUNC_SMBUS_PROC_CALL,
                            I2C_FUNC_SMBUS_READ_BLOCK_DATA,
                            I2C_FUNC_SMBUS_WRITE_BLOCK_DATA,
                            I2C_FUNC_SMBUS_READ_I2C_BLOCK,
                            I2C_FUNC_SMBUS_WRITE_I2C_BLOCK,
                            I2C_FUNC_SMBUS_BYTE,
                            I2C_FUNC_SMBUS_BYTE_DATA,
                            I2C_FUNC_SMBUS_WORD_DATA,
                            I2C_FUNC_SMBUS_BLOCK_DATA,
                            I2C_FUNC_SMBUS_I2C_BLOCK,
                            I2C_FUNC_SMBUS_EMUL};

        printf("I2C ADAPTOR FUNCTIONS: %d\n", mCapabilites);

        for (int i = 0 ; i < (sizeof(i2cFuncId) / sizeof(int)) ; ++i)
        {
            printf("--- %s: %d\n", i2cFuncIdName[i].c_str(), (mCapabilites & i2cFuncId[i] ? 1 : 0));
        }

        printf("\n");
    }
    else
    {
        printf("ERROR: device is not open\n");
    }
}
