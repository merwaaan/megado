#pragma once

#include <stdint.h>

struct M68k;

typedef struct {
    uint8_t* memory;
    struct M68k* m68k;
} Genesis;

Genesis* genesis_make();
void genesis_free(Genesis*);

void genesis_load_rom_file(Genesis* g, char* path);
void genesis_load_rom_data(Genesis* g, uint8_t* data);

// We have to use those to get pointers to different fields of the Genesis struct
// TODO really necessary? Can't we just compute pointer offsets on the JS side?
uint8_t* genesis_memory(Genesis* g) { return g->memory; }
uint8_t* genesis_m68k(Genesis* g) { return g->m68k; }

void genesis_step(Genesis* g);
