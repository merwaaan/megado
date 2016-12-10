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
void genesis_load_rom_data(Genesis* g, uint8_t data);

void genesis_step();
