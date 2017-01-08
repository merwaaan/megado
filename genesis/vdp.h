#pragma once

#include <stdint.h>

typedef struct Vdp
{
    uint8_t* vram;
    uint8_t* cram;
} Vdp;

Vdp* vdp_make();
void vdp_free(Vdp*);
