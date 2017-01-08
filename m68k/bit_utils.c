#include "bit_utils.h"

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
    }

    return 0;
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

char* bin_tostring(int x)
{
    if (x == 0)
        return "0";

    // Count the number of bits
    int x2 = x;
    int bits = 0;
    while (x2 != 0)
    {
        x2 >>= 1;
        ++bits;
    }

    // Build the string bit by bit
    char buffer[33];
    for (int i = 0; i < bits; ++i)
        buffer[i] = BIT(x, bits - i - 1) + '0';
    buffer[bits] = '\0';

    return buffer;
}