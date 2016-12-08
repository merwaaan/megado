#include <m68k/m68k.h>
#include <stdio.h>
#include <stdlib.h>

#include "genesis.h"

Genesis* genesis_make()
{
    Genesis* g = calloc(1, sizeof(Genesis*));
    g->memory = calloc(0x1000000, sizeof(uint32_t));
    g->m68k = m68k_make();
    return g;
}

void genesis_free(Genesis* g)
{
    m68k_free(g->m68k);
    free(g->memory);
    free(g);
}

void genesis_load_rom(Genesis* g, char* path)
{
    FILE* file = fopen(path, "rb");
    if (!file)
    {
        printf("Cannot read file \"%s\"", path);
        return;
    }

    fseek(file, 0, SEEK_END);
    int file_length = ftell(file);
    fseek(file, 0, SEEK_SET);

    fread(g->memory, sizeof(uint32_t), 0x1000000, file);

    fclose(file);
}
