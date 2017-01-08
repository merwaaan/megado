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

uint8_t vdp_read_data_hi(Vdp* v)
{
    return 0; // TODO
}

uint8_t vdp_read_data_lo(Vdp* v)
{
    return 0; // TODO
}

void vdp_write_data(Vdp* v, uint16_t value)
{
    // TODO
}

uint16_t vdp_read_control(Vdp* v)
{
    return 0; // TODO return status
}

void vdp_write_control(Vdp* v, uint16_t value)
{
    // TODO see https://sourceforge.net/p/dgen/dgen/ci/master/tree/vdp.cpp for cancelling commands

    // Register set
    if ((value & 0xE000) == 0x8000)
    {
        uint8_t reg = FRAGMENT(value, 12, 8);
        uint8_t reg_value = WORD_LO(value);

        switch (reg)
        {
        case 0:
            v->hblank_enabled = BIT(value, 4);
            return;

        case 1:
            // TODO vdp enabled here too??
            v->vblank_enabled = BIT(value, 5);
            v->dma_enabled = BIT(value, 4);
            return;

        case 2:
            v->plane_a_nametable = FRAGMENT(value, 5, 3);
            return;

        case 3:
            v->window_nametable = FRAGMENT(value, 5, 1);
            return;

        case 4:
            v->plane_b_nametable = FRAGMENT(value, 2, 0);
            return;

        case 5:
            v->sprites_location = FRAGMENT(value, 6, 0);
            return;

        case 7:
            v->background_color_palette = FRAGMENT(value, 5, 4);
            v->background_color_entry = FRAGMENT(value, 3, 0);
            return;

        case 0xB:
            v->vertical_scrolling = BIT(value, 2);
            v->horizontal_scrolling = FRAGMENT(value, 1, 0);
            return;

        case 0xC:
            v->display_width = BIT(value, 7);
            v->shadow_highlight_enabled = BIT(value, 3);
            v->interlace = FRAGMENT(value, 2, 1);
            return;

        case 0xD:
            v->horizontal_scrolltable = FRAGMENT(value, 5, 0);
            return;

        case 0xF:
            v->auto_increment = value;
            return;

        case 0x10:
            v->vertical_plane_size = FRAGMENT(value, 5, 4);
            v->horizontal_plane_size = FRAGMENT(value, 1, 0);
            return;

        case 0x11:
            v->window_plane_horizontal_direction = BIT(value, 7);
            v->window_plane_horizontal_offset = FRAGMENT(value, 4, 0);
            return;

        case 0x12:
            v->window_plane_vertical_direction = BIT(value, 7);
            v->window_plane_vertical_offset = FRAGMENT(value, 4, 0);
            return;

        case 0x13:
            v->dma_length = (v->dma_length & 0xFFFF0000) | value;
            return;
        case 0x14:
            v->dma_length = (v->dma_length & 0x0000FFFF) | (value << 16);
            return;

        case 0x15: // TODO
            return;
        case 0x16:
            return;
        case 0x17:
            return;
        }
    }
    // Data address set
    else
    {

    }
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


