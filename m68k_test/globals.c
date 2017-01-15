#include <stdlib.h>

#include "globals.h"

M68k* m = NULL;
uint8_t* memory = NULL;

void setup()
{
    memory = calloc(0x1000000, sizeof(uint8_t));
    m = m68k_make();
}

void teardown()
{
    m68k_free(m);
    free(memory);
}
