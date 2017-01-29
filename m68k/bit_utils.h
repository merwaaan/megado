#pragma once

#include <stdint.h>

// The "all bits on" mask is 64 bit to avoid issues due to 
// 32 bit shifts on 32 bit values being undefined behaviour
#define FULL_MASK ((uint64_t) 0xFFFFFFFF)

// Get the n-th bit of x
#define BIT(x, n) (((x) & (1 << (n))) != 0)

// Set/Clear the n-th bit of x
#define BIT_SET(x, n) ((x) | (1 << (n)))
#define BIT_CLR(x, n) ((x) & ~(1 << (n)))

// Change the n-th bit of x to b
#define BIT_CHG(x, n, b) ((x) ^ (((-(b)) ^ (x)) & (1 << (n))))

// Get the bits of x within [start, end]
#define FRAGMENT(x, start, end) (((x) & ~(FULL_MASK << ((start) + 1))) >> (end))

// Mask the bits below/above bit n
#define MASK_BELOW(x, n) ((x) & (FULL_MASK << (n)))
#define MASK_ABOVE(x, n) ((x) & ~(FULL_MASK << ((n) + 1)))

// Mask the bits below/above bit n (inclusive)
#define MASK_BELOW_INC(x, n) ((x) & (FULL_MASK << ((n) + 1)))
#define MASK_ABOVE_INC(x, n) ((x) & ~(FULL_MASK << (n)))

// Get low/high nibble/byte/word
#define NIBBLE_LO(b) ((b) & 0xF)
#define NIBBLE_HI(b) (((b) & 0xF0) >> 4) 
#define BYTE_LO(b) ((b) & 0xFF)
#define BYTE_HI(b) (((b) & 0xFF00) >> 8) 
#define WORD_LO(b) ((b) & 0xFFFF)
#define WORD_HI(b) (((b) & 0xFFFF0000) >> 16) 

#define MAX_VALUE(size) ((size) == Byte ? 0xFF : ((size) == Word ? 0xFFFF : 0xFFFFFFFF))

typedef enum
{
    Byte = 8,
    Word = 16,
    Long = 32
} Size;

// Give the byte count for each size
uint8_t size_in_bytes(Size);

// Return the decimal value of a string representing a base-2 number
uint16_t bin_parse(char* bin);

// Return the binary representation of a number
char* bin_tostring(int x);
