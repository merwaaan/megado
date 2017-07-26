#pragma once

#include <stdint.h>

// The "all bits on" mask is 64 bit to avoid issues due to 
// 32 bit shifts on 32 bit values being undefined behaviour
#define FULL_MASK ((uint64_t) 0xFFFFFFFF)

// Get the n-th bit of x
#define BIT(x, n) (((x) >> (n)) & 1)

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

#define SIGN_BIT(x, size) BIT(x, size - 1)

#define SIGN_EXTEND_B(x) (((x) & 0xFF) | (BIT((x), 7) ? 0xFF00 : 0)) // Byte -> word
#define SIGN_EXTEND_W(x) (((x) & 0xFFFF) | (BIT((x), 15) ? 0xFFFF0000 : 0)) // Word -> long
#define SIGN_EXTEND_B_L(x) (((x) & 0xFF) | (BIT((x), 7) ? 0xFFFFFFFFFFFFFF00 : 0)) // Byte -> long

// http://teaching.idallen.com/dat2343/10f/notes/040_overflow.txt
#define CHECK_CARRY_ADD(a, b, size) (MASK_ABOVE_INC((a) + (b), (size)) < (a))
#define CHECK_CARRY_SUB(a, b, size) (MASK_ABOVE_INC((a) - (b), (size)) > (a))
#define CHECK_OVERFLOW_ADD(a, b, size) (SIGN_BIT(a, size) == SIGN_BIT(b, size) && SIGN_BIT(a, size) != SIGN_BIT((a) + (b), size))
#define CHECK_OVERFLOW_SUB(a, b, size) ((!SIGN_BIT(a, size) && SIGN_BIT(b, size) && SIGN_BIT((a) - (b), size)) || (SIGN_BIT(a, size) && !SIGN_BIT(b, size) && !SIGN_BIT((a) - (b), size)))

// Modulo supporting negative numbers
#define UMOD(A, B) (((A) % (B) + (B)) % (B))

typedef enum
{
    Byte = 8,
    Word = 16,
    Long = 32,

    InvalidSize = -1
} Size;

// Give the byte count for each size
uint8_t size_in_bytes(Size);

// Return the decimal value of a string representing a base-2 number
uint16_t bin_parse(char* bin);

// Return the binary representation of a number
char* bin_tostring(int value, char* buffer);
