#pragma once

#include <stdint.h>

// Get the nth bit of x
#define BIT(x, n) ((x & (1 << n)) != 0)

// Get the bits of x from start to end
#define FRAGMENT(x, start, end) ((x & ~(0xFFFF << start)) >> end)

uint16_t *_memory;
