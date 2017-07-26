#include <stdio.h>
#include <stdlib.h>

#include "bit_utils.h"
#include "../utils.h"

uint8_t size_in_bytes(Size size)
{
    switch (size)
    {
    case Byte:
        return 1;
    case Word:
        return 2;
    case Long:
        return 4;
    case InvalidSize:
    default:
        FATAL("Invalid size: %x", size);
    }
}

uint16_t bin_parse(char* bin)
{
    uint16_t value = 0;

    while (*bin) {
        if (*bin == '1' || *bin == '0')
            value = (value << 1) + (*bin - '0');
        ++bin;
    }

    return value;
}

char* bin_tostring(int value, char* buffer)
{
    if (value == 0)
    {
        buffer[0] = '0';
        buffer[1] = '\0';
        return buffer;
    }

    // Count the number of bits
    int x2 = value;
    int bits = 0;
    while (x2 != 0)
    {
        x2 >>= 1;
        ++bits;
    }

    // Build the string bit by bit
    for (int i = 0; i < bits; ++i)
        buffer[i] = BIT(value, bits - i - 1) + '0';
    buffer[bits] = '\0';

    return buffer;
}
