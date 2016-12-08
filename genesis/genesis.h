#pragma once

#include <stdint.h>

struct M68k;

typedef struct {
    uint16_t* memory;
    struct M68k* m68k;
} Genesis;

Genesis* genesis_make();
void genesis_free(Genesis*);

void genesis_load_rom(Genesis* g, char* path);
