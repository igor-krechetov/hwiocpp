#include "DeviceI2C.hpp"
#include <string>
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
    if (true == isOpen())
    {
        close(mFD);
        mFD = INVALID_FD;
        mCapabilites = 0;
    }
}

byte DeviceI2C::readByte()
{
    byte result = 0;

    if (true == isOpen())
    {
        result = i2c_smbus_read_byte(mFD);
    }

    return result;
}

byte DeviceI2C::readByte(const char cmd)
{
    byte result = 0;

    if (true == isOpen())
    {
        result = i2c_smbus_read_byte_data(mFD, cmd);
    }

    return result;
}

int DeviceI2C::readBuffer(byte* outBuffer, const size_t bytesCount)
{
    printf("READ: %lu bytes\n", bytesCount);
    int actualBytes = 0;

    if (true == isOpen())
    {
        actualBytes = read(mFD, outBuffer, bytesCount);
    }

    return actualBytes;
}

bool DeviceI2C::writeBuffer(const std::vector<byte>& buffer)
{
    return writeBuffer(buffer.data(), buffer.size());
}

bool DeviceI2C::writeBuffer(const byte* buffer, const size_t bytesCount)
{
    printf("WRITE: %lu bytes\n", bytesCount);
    bool result = false;

    if (true == isOpen())
    {
        write(mFD, buffer, bytesCount);
        result = true;
    }

    return result;
}

bool DeviceI2C::writeBuffer(const char cmd, const byte* buffer, const size_t bytesCount)
{
    printf("WRITE: %lu bytes\n", bytesCount);
    bool result = false;

    if (true == isOpen())
    {
        // TODO: use different API depending on capabilities
        result = (0 == i2c_smbus_write_block_data(mFD, cmd, bytesCount, buffer));
    }

    return result;
}

bool DeviceI2C::writeBuffer(const char cmd, const std::vector<byte>& buffer)
{
    return writeBuffer(cmd, buffer.data(), buffer.size());
}

void DeviceI2C::printCapabilities()
{
    if (true == isOpen())
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
