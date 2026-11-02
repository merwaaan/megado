#include <stdlib.h>

#include "globals.h"

Genesis* g = NULL;
M68k* m = NULL;
uint8_t* memory = NULL;

void setup(void* data)
{
    memory = calloc(0x1000000, sizeof(uint8_t));
    g = genesis_make();
    m = g->m68k;
    test_map_init(g->map);
    m68k_initialize(m);
}

void teardown(void* data)
{
    genesis_free(g);
    free(memory);
}
