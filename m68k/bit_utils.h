#pragma once

#include <stdint.h>

// Get the nth bit of x
#define BIT(x, n) ((x & (1 << n)) != 0)

// Get the bits of x in [start, end]
#define FRAGMENT(x, start, end) ((x & ~(0xFFFF << start + 1)) >> end)

uint16_t parse_bin(char* bin);
