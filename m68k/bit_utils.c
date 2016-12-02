#include "bit_utils.h"

uint16_t parse_bin(char* bin)
{
    uint16_t value = 0;

    while (*bin) {
        if (*bin == '1' || *bin == '0')
            value = (value << 1) + (*bin - '0');
        ++bin;
    }

    return value;
}
