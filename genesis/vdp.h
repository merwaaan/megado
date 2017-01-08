#pragma once

#include <m68k/bit_utils.h>
#include <SDL.h>
#include <stdint.h>

// Extract color components from a palette word
#define RED(c) FRAGMENT((c), 3, 1)
#define GREEN(c) FRAGMENT((c), 7, 5)
#define BLUE(c) FRAGMENT((c), 11, 9)

// Convert 3-bit components to 8-bit
#define COLOR_8(c) (c) * 31
#define RED_8(c) COLOR_8(RED(c))
#define GREEN_8(c) COLOR_8(GREEN(c))
#define BLUE_8(c) COLOR_8(BLUE(c))

typedef struct Vdp
{
    uint8_t* vram;
    uint16_t* cram;

    SDL_Window* window;
    SDL_Renderer* renderer;
} Vdp;

Vdp* vdp_make();
void vdp_free(Vdp*);

void vdp_draw(Vdp*);
