#include <stdlib.h>

#include "vdp.h"

Vdp* vdp_make()
{
    Vdp* v = calloc(1, sizeof(Vdp));
    v->vram = calloc(0x10000, sizeof(uint8_t));
    v->cram = calloc(64, sizeof(uint16_t));
    return v;
}

void vdp_free(Vdp* v)
{
    free(v->vram);
    free(v->cram);
    free(v);
}
