#include <m68k/m68k.h>
#include <stdlib.h>
#include <stdio.h>

#include "vdp.h"

Vdp* vdp_make(M68k* cpu)
{
    Vdp* v = calloc(1, sizeof(Vdp));
    v->cpu = cpu;
    v->vram = calloc(0x10000, sizeof(uint8_t));
    v->cram = calloc(64, sizeof(uint16_t));
    v->vsram = calloc(40, sizeof(uint16_t));
    v->pending_command = false;

    // Initialize SDL
    /*if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        printf("An error occurred while initializing SDL: %s", SDL_GetError());
    }
    else
    {
        SDL_CreateWindowAndRenderer(640, 480, SDL_WINDOW_OPENGL, &v->window, &v->renderer);
    }*/

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
    if (v == NULL)
        return;

    /*SDL_DestroyWindow(v->window);
    SDL_DestroyRenderer(v->renderer);
    SDL_Quit();*/

    free(v->vram);
    free(v->cram);
    free(v->vsram);
    free(v);
}

uint8_t vdp_read_data_hi(Vdp* v)
{
    return 0; // TODO
}

uint8_t vdp_read_data_lo(Vdp* v)
{
    v->pending_command = false;

    return 0; // TODO
}

void vdp_write_data(Vdp* v, uint16_t value)
{
    v->pending_command = false;

    // DMA transfers
    if (v->dma_enabled)
    {
        // VRAM fill
        if (v->dma_type == 2)
        {
            // TODO do not freeze during the transfer
            v->dma_in_progress = true;

            int from = v->dma_address;
            int to = v->dma_address + v->dma_length;
            for (int i = from; i < to; i += v->auto_increment)
            {
                v->vram[i] = NIBBLE_HI(value); // TODO optim, compute hi/lo once
                v->vram[i + 1] = NIBBLE_LO(value);
            }

            v->dma_in_progress = false;
        }
    }
    else
    {
        // CRAM write
        if (v->access_mode == 3)
        {
            v->cram[v->access_address] = value;
            v->access_address += v->auto_increment;
        }
        // VSRAM write
        else if (v->access_mode == 5)
        {
            v->vsram[v->access_address] = value;
            v->access_address += v->auto_increment;
        }
        else {
            printf("");
        }
    }
}

uint16_t vdp_read_control(Vdp* v)
{
    v->pending_command = false;
    // TODO
    return
        0x3400 |
        (false << 9) | // FIFO not empty
        (false << 8) | // FIFO not full
        (false << 7) | // Vertical interrupt occurred
        (false << 6) | // Sprite overflow
        (false << 5) | // Sprite collision
        (false << 4) | // Odd frame
        (false << 3) | // Vertical blank
        (false << 2) | // Horizontal blank
        (v->dma_in_progress << 1) | // DMA transfer currently executing
        false;        // NTSC (0) / PAL (1)
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
            v->hblank_enabled = BIT(reg_value, 4);
            v->hv_counter_enabled = !BIT(reg_value, 1);
            return;

        case 1:
            v->display_enabled = BIT(reg_value, 6);
            v->vblank_enabled = BIT(reg_value, 5);
            v->dma_enabled = BIT(reg_value, 4);
            v->display_mode = BIT(reg_value, 3);
            return;

        case 2:
            v->plane_a_nametable = FRAGMENT(reg_value, 5, 3);
            return;

        case 3:
            v->window_nametable = FRAGMENT(reg_value, 5, 1);
            return;

        case 4:
            v->plane_b_nametable = FRAGMENT(reg_value, 2, 0);
            return;

        case 5:
            v->sprites_attributetable = FRAGMENT(reg_value, 6, 0);
            return;

        case 7:
            v->background_color_palette = FRAGMENT(reg_value, 5, 4);
            v->background_color_entry = FRAGMENT(reg_value, 3, 0);
            return;

        case 0xA:
            v->hblank_counter = reg_value;
            return;

        case 0xB:
            v->vertical_scrolling = BIT(reg_value, 2);
            v->horizontal_scrolling = FRAGMENT(reg_value, 1, 0);
            return;

        case 0xC:
            v->display_width = BIT(reg_value, 7); // Should be same value as bit 0
            v->shadow_highlight_enabled = BIT(reg_value, 3);
            v->interlace_mode = FRAGMENT(reg_value, 2, 1);
            return;

        case 0xD:
            v->horizontal_scrolltable = FRAGMENT(reg_value, 5, 0);
            return;

        case 0xF:
            v->auto_increment = reg_value;
            return;

        case 0x10:
            v->vertical_plane_size = FRAGMENT(reg_value, 5, 4); // TODO Convert to size
            v->horizontal_plane_size = FRAGMENT(reg_value, 1, 0); // TODO
            return;

        case 0x11:
            v->window_plane_horizontal_direction = BIT(reg_value, 7);
            v->window_plane_horizontal_offset = FRAGMENT(reg_value, 4, 0);
            return;

        case 0x12:
            v->window_plane_vertical_direction = BIT(reg_value, 7);
            v->window_plane_vertical_offset = FRAGMENT(reg_value, 4, 0);
            return;

        case 0x13:
            v->dma_length = (v->dma_length & 0xFF00) | reg_value;
            return;
        case 0x14:
            v->dma_length = (v->dma_length & 0x00FF) | (reg_value << 8);
            return;

        case 0x15:
            v->dma_address = (v->dma_address & 0xFFFF00) | reg_value;
            return;
        case 0x16:
            v->dma_address = (v->dma_address & 0xFF00FF) | (reg_value << 8);
            return;
        case 0x17:
            v->dma_address = (v->dma_address & 0x00FFFF) | ((reg_value & 0x3F) << 16);
            v->dma_type = FRAGMENT(reg_value, 7, 6); // TODO convert    
            return;
        }
    }
    // Data address set
    else
    {
        if (!v->pending_command)
        {
            v->access_mode = (v->access_mode & 0xFC) | FRAGMENT(value, 15, 14); // B15-14 -> B1-0 of access mode
            v->access_address = (v->access_address & 0xC000) | FRAGMENT(value, 13, 0); // B13-0 -> B13-0 of address

            v->pending_command = true;
        }
        else
        {
            v->access_mode = (v->access_mode & 3) | (FRAGMENT(value, 7, 4) << 2); // B7-4 -> B5-2 of access mode
            v->access_address = (v->access_address & 0x3FFF) | (FRAGMENT(value, 1, 0) << 14); // B1-0 -> B15-14 of address

            v->pending_command = false;

            // Memory to VRAM transfer
            // TODO confused. difference bw dma type & access mode?
            if (v->dma_enabled && v->dma_type == 1)
            {
                // to CRAM
                if ((v->access_mode & 0x1F) == 3)
                {
                    // TODO do not freeze during the transfer
                    v->dma_in_progress = true;

                    int from = v->dma_address;
                    int to = v->dma_address + v->dma_length;
                    for (int i = from; i < to; i += v->auto_increment)
                        v->cram[i] = m68k_read_w(v->cpu, i);

                    v->dma_in_progress = false;
                }
                // to VRAM
                if ((v->access_mode & 0x1F) == 1)
                {
                    // TODO do not freeze during the transfer
                    v->dma_in_progress = true;

                    for (int i = 0; i < v->dma_length; i += v->auto_increment)
                    {
                        int source = v->dma_address + i;
                        uint8_t value = m68k_read_w(v->cpu, source);

                        int destination = v->access_address + i;
                        v->vram[destination] = NIBBLE_HI(value); // TODO optim, compute hi/lo once
                        v->vram[destination + 1] = NIBBLE_LO(value);
                    }

                    v->dma_in_progress = false;
                }
            }
        }
    }
}

int pixel = 0;
int line = 0;

void vdp_draw(Vdp* v)
{
    ++pixel;
    if (pixel > 480)
    {
        pixel = 0;
        ++line;

        if (v->hblank_enabled)
            m68k_request_interrupt(v->cpu, HBLANK_IRQ);
    }
    if (line > 320)
    {
        line = 0;

        if (v->vblank_enabled)
            m68k_request_interrupt(v->cpu, VBLANK_IRQ);
    }

    // State of the art emulation accuracy


    /*if (v->renderer == NULL)
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

    SDL_RenderPresent(v->renderer);*/
}


