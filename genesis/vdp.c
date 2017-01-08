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
        SDL_CreateWindowAndRenderer(640, 480, SDL_WINDOW_OPENGL, &v->window, &v->renderer);
    }

    // Random colors 
    v->cram[0] = 0xFFFF;
    v->cram[1] = 0xFF00;
    v->cram[2] = 0x0E00;
    v->cram[8] = 0xF81F;
    v->cram[9] = 0x1100;
    v->cram[10] = 0xC0FC;
    v->cram[20] = 0x000F;
    v->cram[21] = 0xABCD;
    v->cram[40] = 0x1234;

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

void vdp_draw(Vdp* v)
{
    if (v->renderer == NULL)
        return;

    SDL_SetRenderDrawColor(v->renderer, 0, 0, 0, 255);
    SDL_RenderClear(v->renderer);

    // Draw the color palette
    SDL_Rect cell = { 0, 0, 10, 10 };
    for (int line = 0; line < 4; ++line)
    {
        for (int col = 0; col < 16; ++col)
        {
            uint16_t color = v->cram[line * 16 + col];
            SDL_SetRenderDrawColor(v->renderer, RED_8(color), GREEN_8(color), BLUE_8(color), 255);
            SDL_RenderFillRect(v->renderer, &cell);

            cell.x += 10;
        }

        cell.x = 0;
        cell.y += 10;
    }

    SDL_RenderPresent(v->renderer);
}


