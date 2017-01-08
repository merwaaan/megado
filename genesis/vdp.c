#include <stdlib.h>
#include <stdio.h>

#include "vdp.h"

Vdp* vdp_make()
{
    Vdp* v = calloc(1, sizeof(Vdp));
    v->vram = calloc(0x10000, sizeof(uint8_t));
    v->cram = calloc(64, sizeof(uint16_t));

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        printf("An error occurred while initializing SDL: %s", SDL_GetError());
    }
    else
    {
        SDL_CreateWindowAndRenderer(640, 480, 0, &v->window, &v->renderer);
    }

    return v;
}

void vdp_free(Vdp* v)
{
    SDL_DestroyWindow(v->window);
    SDL_DestroyRenderer(v->renderer);
    SDL_Quit();

    free(v->vram);
    free(v->cram);
    free(v);
}
