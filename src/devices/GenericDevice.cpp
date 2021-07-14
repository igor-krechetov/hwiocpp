#include "GenericDevice.hpp"
#include <unistd.h>
#include <cstdio>

GenericDevice::GenericDevice()
{
    int i = 1;

    if (reinterpret_cast<char*>(&i)[0] == 1)
    {
        mNativeBytesOrder = Endianness::LITTLE;
    }
    else
    {
        mNativeBytesOrder = Endianness::BIG;
    }

    mBuffer.resize(4, 0);
}

void GenericDevice::wait(const unsigned int milliseconds)
{
    printf("WAIT: %u ms...\n", milliseconds);
    usleep(milliseconds * 1000);
}

double GenericDevice::remap(double value, double oldMin, double oldMax, double newMin, double newMax)
{
    bool isReverse = false;

    if (oldMin > oldMax)
    {
        double temp = oldMin;

        oldMin = oldMax;
        oldMax = temp;
        isReverse = true;
    }

    double normValue = value - oldMin;
    double oldRange = oldMax - oldMin;
    double newRange = newMax - newMin;
    double valuePercent = normValue / oldRange;
    double newValue = 0;

    if (isReverse)
    {
        newValue = (newMax + newMin) - (newMin + valuePercent * newRange);
    }
    else
    {
        newValue = (newMin + valuePercent * newRange);
    }

    return newValue;
}

uint16_t GenericDevice::normalizeBytes(const uint16_t value, const Endianness expectedOrder)
{
    if (expectedOrder != mNativeBytesOrder)
    {
        // return ((value >> 8) & 0xFF) | ((value << 8) & 0xFF00);
        return (GET_BYTE0(value) << 8) | GET_BYTE1(value);
    }

    return value;
}

uint32_t GenericDevice::normalizeBytes(const uint32_t value, const Endianness expectedOrder)
{
    if (expectedOrder != mNativeBytesOrder)
    {
        // return ((value >> 8) & 0xFF) | ((value << 8) & 0xFF00);
        return (GET_BYTE0(value) << 24) |
               (GET_BYTE1(value) << 16) |
               (GET_BYTE2(value) << 8) |
               GET_BYTE3(value);
    }

    return value;
}

const byte* GenericDevice::normalizeBytes(const byte* buffer, const size_t bytesCount, const Endianness expectedOrder)
{
    const byte* result = buffer;

    if (expectedOrder != mNativeBytesOrder)
    {
        if (mBuffer.size() < bytesCount)
        {
            mBuffer.resize(bytesCount);
        }

        for (int i = 0 ; i < bytesCount; ++i)
        {
            mBuffer[bytesCount - 1 - i] = buffer[i];
        }
        
        result = mBuffer.data();
    }

    return result;
}