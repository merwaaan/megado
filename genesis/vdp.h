#pragma once

#include <SDL.h>
#include <stdint.h>

typedef struct Vdp
{
    uint8_t* vram;
    uint8_t* cram;

    SDL_Window* window;
    SDL_Renderer* renderer;
} Vdp;

Vdp* vdp_make();
void vdp_free(Vdp*);
