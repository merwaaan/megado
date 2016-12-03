#pragma once

#include <stdint.h>

// Get the nth bit of x
#define BIT(x, n) (((x) & (1 << (n))) != 0)

// Get the bits of x in [start, end]
#define FRAGMENT(x, start, end) (((x) & ~(0xFFFF << (start) + 1)) >> (end))

// Mask the bits below/above bit n
#define MASK_BELOW(x, n) ((x) & (0xFFFF << (n)))
#define MASK_ABOVE(x, n) ((x) & ~(0xFFFF << (n)))

// Mask the bits below/above bit n (inclusive)
#define MASK_BELOW_INC(x, n) ((x) & (0xFFFF << (n + 1)))
#define MASK_ABOVE_INC(x, n) ((x) & ~(0xFFFF << (n - 1)))

// Return the decimal value of a string representing a base-2 number
uint16_t parse_bin(char* bin);
