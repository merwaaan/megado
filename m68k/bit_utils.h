#pragma once

#include <stdint.h>

// Get the n-th bit of x
#define BIT(x, n) (((x) & (1 << (n))) != 0)

// Set the n-th bit of x to b
#define BIT_SET(x, n, b) x = (x) ^ (((-b) ^ (x)) & (1 << (n)));

// Get the bits of x in [start, end]
#define FRAGMENT(x, start, end) (((x) & ~(0xFFFFFFFF << ((start) + 1))) >> (end))

// Mask the bits below/above bit n
#define MASK_BELOW(x, n) ((x) & (0xFFFFFFFFF << (n)))
#define MASK_ABOVE(x, n) ((x) & ~(0xFFFFFFFFF << ((n) + 1)))

// Mask the bits below/above bit n (inclusive)
#define MASK_BELOW_INC(x, n) ((x) & (0xFFFFFFFFF << ((n) + 1)))
#define MASK_ABOVE_INC(x, n) ((x) & ~(0xFFFFFFFFF << (n)))

// Return the decimal value of a string representing a base-2 number
uint16_t parse_bin(char* bin);
