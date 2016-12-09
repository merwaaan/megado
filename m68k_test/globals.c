#include <stdlib.h>

#include "globals.h"

M68k* m = NULL;

void setup()
{
    m = m68k_make();
    m->memory = calloc(0x1000000, sizeof(uint8_t));
}

void teardown()
{
    free(m->memory);
    m68k_free(m);
}
