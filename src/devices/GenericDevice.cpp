#include "GenericDevice.hpp"
#include <unistd.h>
#include <cstdio>

void GenericDevice::wait(const unsigned int milliseconds)
{
    printf("WAIT: %u ms...\n", milliseconds);
    usleep(milliseconds * 1000);
}
