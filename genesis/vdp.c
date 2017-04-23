#include <assert.h>
#include <m68k/m68k.h>
#include <stdlib.h>
#include <stdio.h>

#include "vdp.h"

#if true
#define LOG_VDP(...) printf(__VA_ARGS__)
#else
#define LOG_VDP(...)
#endif

void draw(Vdp* v);

// Black & white debug palette
uint16_t debug_palette[16] = {
    0,
    0x222,
    0x444,
    0x666,
    0x888,
    0xaaa,
    0xccc,
    0xeee,
    // Cannot represent more than 8 greyscale values with 3 bits per components
    0x10,
    0x100,
    0x1000,
    0x110,
    0x1010,
    0x1100,
    0x1084,
    0x490
};

// Plane size codes for register 0x10
uint8_t plane_size_codes[] = { 32, 64, 0, 128 };

Vdp* vdp_make(M68k* cpu)
{
    Vdp* v = calloc(1, sizeof(Vdp));
    v->cpu = cpu;
    v->vram = calloc(0x10000, sizeof(uint8_t));
    v->cram = calloc(64, sizeof(uint16_t));
    v->vsram = calloc(40, sizeof(uint16_t));
    v->pending_command = false;

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        printf("An error occurred while initializing SDL: %s", SDL_GetError());
    }
    else
    {
        SDL_CreateWindowAndRenderer(1500, 1000, SDL_WINDOW_OPENGL, &v->window, &v->renderer);
    }

    for (int i = 0; i < 64; ++i)
        v->cram[i] = 0xFFFF;

    return v;
}

void vdp_free(Vdp* v)
{
    if (v == NULL)
        return;

    SDL_DestroyWindow(v->window);
    SDL_DestroyRenderer(v->renderer);
    SDL_Quit();

    free(v->vram);
    free(v->cram);
    free(v->vsram);
    free(v);
}

uint16_t vdp_read_data(Vdp* v)
{
    v->pending_command = false;

    uint16_t value = 0;
    switch (v->access_mode & 0xF) // TODO do it if DMA bit on? (& 7 / & 3f) 
    {
    case 0: // VRAM read

        LOG_VDP("Reading VRAM @ %04x\n", v->access_address);

        value = v->vram[v->access_address] << 8 | v->vram[v->access_address + 1];
        break;

    case 8: // CRAM read

        LOG_VDP("Reading CRAM @ %02x\n", v->access_address >> 1);

        value = v->cram[v->access_address >> 1 & 0x3F];
        break;

    case 4: // VSRAM read

        LOG_VDP("Reading VSRAM @ %02x\n", v->access_address >> 1);

        value = v->vsram[v->access_address >> 1 & 0x3F];
        break;

    default:
        printf("WARNING! Unhandled access mode\n");
    }

    v->access_address += v->auto_increment;

    return value;
}

void vdp_write_data(Vdp* v, uint16_t value)
{
    //printf("[%06x] data write: %02x\n", v->cpu->instruction_address, value);

    v->pending_command = false;

    switch (v->access_mode & 0xF) // TODO do it if DMA bit on? (& 7 / & 3f) 
    {
    case 1: // VRAM write

        LOG_VDP("\tWrite %04x to VRAM @ %04x\n", value, v->access_address);

        v->vram[v->access_address] = BYTE_HI(value); // TODO sure about that?
        v->vram[v->access_address ^ 1] = BYTE_LO(value);
        v->access_address += v->auto_increment;
        break;

    case 3: // CRAM write

        LOG_VDP("\tWrite %02x to CRAM @ %02x\n", value, v->access_address >> 1);

        v->cram[v->access_address >> 1 & 0x3F] = value;
        v->access_address += v->auto_increment;
        break;

    case 5: // VSRAM write

        LOG_VDP("\tWrite %02x to VSRAM @ %02x\n", value, v->access_address >> 1);

        //assert((v->access_address >> 1) < 0x28);
        v->vsram[v->access_address >> 1 & 0x28] = value;
        v->access_address += v->auto_increment;
        break;

    default:
        printf("WARNING! Unhandled access mode\n");
    }

    // Handle pending DMA fills
    if (v->dma_enabled && v->pending_dma_fill)
    {
        v->pending_dma_fill = false;

        switch (v->access_mode & 0xF)
        {
        case 1: // VRAM fill

            /* TODO not sure about that
            // The lower byte is written to the address specified
            v->vram[v->access_address] = NIBBLE_LO(value);

            // The upper byte is written to the adjacent addresses
            uint8_t hi = NIBBLE_HI(value);
            for (int i = 0; i < v->dma_length; ++i)
            {
            assert((v->access_address ^ 1) < 0x10000);
            v->vram[v->access_address ^ 1] = hi;
            v->access_address += v->auto_increment;
            }*/

            LOG_VDP("\tDMA Fill to VRAM @ %04x, value %04x, length %04x, auto increment %04x\n", v->access_address, value, v->dma_length, v->auto_increment);

            uint8_t hi = BYTE_HI(value);
            do
            {
                v->vram[v->access_address ^ 1] = hi;
                v->access_address += v->auto_increment;

                // The DMA source address is not used in this process but must be incremented anyway
                ++v->dma_source_address_lo;

            } while (--v->dma_length);

            break;

        case 3: // CRAM fill

            LOG_VDP("\tDMA Fill to CRAM @ %04x, value %04x, length %04x, auto increment %04x\n", v->access_address >> 1, value, v->dma_length, v->auto_increment);

            do {
                v->cram[v->access_address >> 1 & 0x3F] = value;
                v->access_address += v->auto_increment;

                ++v->dma_source_address_lo;

            } while (--v->dma_length);
            break;

        case 5: // VSRAM fill

            LOG_VDP("\tDMA Fill to VSRAM @ %04x, value %04x, length %04x, auto increment %04x\n", v->access_address >> 1, value, v->dma_length, v->auto_increment);

            do {
                v->vsram[v->access_address >> 1 & 0x3F] = value;
                v->access_address += v->auto_increment;

                ++v->dma_source_address_lo;

            } while (--v->dma_length);
            break;

        default:
            printf("?");
        }
    }
}

uint16_t vdp_read_control(Vdp* v)
{
    v->pending_command = false;

    // TODO
    return
        0x3400 |
        false << 9 | // FIFO not empty
        false << 8 | // FIFO not full
        false << 7 | // Vertical interrupt occurred
        false << 6 | // Sprite overflow
        false << 5 | // Sprite collision
        false << 4 | // Odd frame
        v->vblank_in_progress << 3 | // Vertical blanking
        v->hblank_in_progress << 2 | // Horizontal blanking
        v->dma_in_progress << 1 | // DMA transfer currently executing
        true;        // NTSC (0) / PAL (1)
}

void vdp_write_control(Vdp* v, uint16_t value)
{
    LOG_VDP("[%0x] control write: %02x\n", v->cpu->instruction_address, value);

    // TODO see https://sourceforge.net/p/dgen/dgen/ci/master/tree/vdp.cpp for cancelling commands

    // Register write
    if ((value & 0xC000) == 0x8000)
    {
        uint8_t reg = FRAGMENT(value, 12, 8);
        uint8_t reg_value = WORD_LO(value);

        LOG_VDP("\tregister %02x, value %02x\n", reg, reg_value);

        switch (reg)
        {
        case 0:
            v->hblank_enabled = BIT(reg_value, 4);
            v->hv_counter_enabled = !BIT(reg_value, 1);

            LOG_VDP("\t\tH-blank enabled %d, HV-counter enabled %d\n", v->hblank_enabled, v->hv_counter_enabled);
            return;

        case 1:
            v->display_enabled = BIT(reg_value, 6);
            v->vblank_enabled = BIT(reg_value, 5);
            v->dma_enabled = BIT(reg_value, 4);
            v->display_mode = BIT(reg_value, 3);

            LOG_VDP("\t\tDisplay enabled %d, V-blank enabled %d, DMA enabled %d, Display mode %d\n", v->display_enabled, v->vblank_enabled, v->dma_enabled, v->display_mode);
            return;

        case 2:
            v->plane_a_nametable = (reg_value & 0x38) * 0x400;

            LOG_VDP("\t\tPlane A nametable %04x\n", v->plane_a_nametable);
            return;

        case 3:
            v->window_nametable = (reg_value & 0x3E) * 0x400;

            LOG_VDP("\t\tWindow nametable %04x\n", v->window_nametable);
            return;

        case 4:
            v->plane_b_nametable = ((reg_value & 7) << 3) * 0x400;

            LOG_VDP("\t\tPlane B nametable %04x\n", v->plane_b_nametable);
            return;

        case 5:
            v->sprites_attributetable = FRAGMENT(reg_value, 6, 0);

            LOG_VDP("\t\tSprites attribute table %04x\n", v->sprites_attributetable);
            return;

        case 7:
            v->background_color_palette = FRAGMENT(reg_value, 5, 4);
            v->background_color_entry = FRAGMENT(reg_value, 3, 0);

            LOG_VDP("\t\tBackground palette %d, entry %d\n", v->background_color_palette, v->background_color_entry);
            return;

        case 0xA:
            v->hblank_counter = reg_value;

            LOG_VDP("\t\tH-blank counter %04x\n", v->hblank_counter);
            return;

        case 0xB:
            v->vertical_scrolling = BIT(reg_value, 2);
            v->horizontal_scrolling = FRAGMENT(reg_value, 1, 0);

            LOG_VDP("\t\tVertical scrolling %d, Horizontal scrolling %d\n", v->vertical_scrolling, v->horizontal_scrolling);
            return;

        case 0xC:
            v->display_width = BIT(reg_value, 7); // Should be same value as bit 0
            v->shadow_highlight_enabled = BIT(reg_value, 3);
            v->interlace_mode = FRAGMENT(reg_value, 2, 1);

            LOG_VDP("\t\tDisplay width %d, Shadow/Highlight enabled %d, Interlace mode  %d\n", v->display_mode, v->shadow_highlight_enabled, v->interlace_mode);
            return;

        case 0xD:
            v->horizontal_scrolltable = FRAGMENT(reg_value, 5, 0);

            LOG_VDP("\t\tHorizontal scrolltable %d\n", v->horizontal_scrolltable);
            return;

        case 0xF:
            v->auto_increment = reg_value;

            LOG_VDP("\t\tAuto-increment %d\n", v->auto_increment);
            return;

        case 0x10:
            v->vertical_plane_size = plane_size_codes[FRAGMENT(reg_value, 5, 4)];
            v->horizontal_plane_size = plane_size_codes[FRAGMENT(reg_value, 1, 0)];

            LOG_VDP("\t\tVertical plane size %d, Horizontal plane size %d\n", v->vertical_plane_size, v->horizontal_plane_size);
            return;

        case 0x11:
            v->window_plane_horizontal_direction = BIT(reg_value, 7);
            v->window_plane_horizontal_offset = FRAGMENT(reg_value, 4, 0);

            LOG_VDP("\t\tHorizontal Window plane direction %d, Window plane offset %d\n", v->window_plane_horizontal_direction, v->window_plane_horizontal_offset);
            return;

        case 0x12:
            v->window_plane_vertical_direction = BIT(reg_value, 7);
            v->window_plane_vertical_offset = FRAGMENT(reg_value, 4, 0);

            LOG_VDP("\t\tVertical window plane direction %d, Window plane offset %d\n", v->window_plane_vertical_offset, v->window_plane_vertical_offset);
            return;

        case 0x13:
            v->dma_length = (v->dma_length & 0xFF00) | reg_value;

            LOG_VDP("\t\tDMA length low %02x (%04x)\n", reg_value, v->dma_length);
            return;
        case 0x14:
            v->dma_length = (v->dma_length & 0x00FF) | (reg_value << 8);

            LOG_VDP("\t\tDMA length high %02x (%04x)\n", reg_value, v->dma_length);
            return;

        case 0x15:
            v->dma_source_address_lo = (v->dma_source_address_lo & 0xFF00) | reg_value;

            LOG_VDP("\t\tDMA source address low %02x (%08x)\n", reg_value, v->dma_source_address_hi << 16 | v->dma_source_address_lo);
            return;
        case 0x16:
            v->dma_source_address_lo = (v->dma_source_address_lo & 0x00FF) | (reg_value << 8);

            LOG_VDP("\t\tDMA source address med %02x (%08x)\n", reg_value, v->dma_source_address_hi << 16 | v->dma_source_address_lo);
            return;
        case 0x17:
            v->dma_type = FRAGMENT(reg_value, 7, 6); // TODO convert

            uint8_t address_mask = v->dma_type > 1 ? 0x3F : 0x7F; // Bit 6 is only part of the address for memory to VRAM DMA transfers
            v->dma_source_address_hi = reg_value & address_mask;

            LOG_VDP("\t\tDMA source address high %02x (%08x), DMA type %04x\n", reg_value, v->dma_source_address_hi << 16 | v->dma_source_address_lo, v->dma_type);
            return;

        case 6:
        case 8:
        case 9:
        case 0xE:
            LOG_VDP("\t\tUnhandled register\n");
            return;
        }
    }
    // Command word
    else
    {
        if (!v->pending_command)
        {
            v->access_mode = (v->access_mode & 0xFC) | FRAGMENT(value, 15, 14); // B15-14 -> B1-0 of access mode
            v->access_address = (v->access_address & 0xC000) | FRAGMENT(value, 13, 0); // B13-0 -> B13-0 of address

            v->pending_command = true;

            LOG_VDP("\tFirst command word %04x: mode %04x, address %04x\n", value, v->access_mode, v->access_address);
        }
        else
        {
            v->access_mode = (v->access_mode & 3) | (FRAGMENT(value, 7, 4) << 2); // B7-4 -> B5-2 of access mode
            v->access_address = (v->access_address & 0x3FFF) | (FRAGMENT(value, 1, 0) << 14); // B1-0 -> B15-14 of address

            v->pending_command = false;

            LOG_VDP("\tSecond command word %04x: mode %04x, address %04x\n", value, v->access_mode, v->access_address);

            // Handle DMA transfers (CD5 set)
            if (v->dma_enabled && v->access_mode & 0x20)
            {
                // Memory to...
                if (v->dma_type < 2)
                {
                    switch (v->access_mode & 0xF)
                    {
                    case 1:
                        // ... VRAM
                        LOG_VDP("\tDMA transfer from %08x to VRAM @ %04x, length %04x, auto increment %04x\n", (v->dma_source_address_hi << 16 | v->dma_source_address_lo) << 1, v->access_address, v->dma_length, v->auto_increment);

                        do {
                            uint16_t value = m68k_read_w(v->cpu, (v->dma_source_address_hi << 16 | v->dma_source_address_lo) << 1);
                            v->vram[v->access_address] = BYTE_HI(value);
                            v->vram[v->access_address ^ 1] = BYTE_LO(value);

                            ++v->dma_source_address_lo;
                            v->access_address += v->auto_increment;
                        } while (--v->dma_length);

                        break;

                    case 3:
                        // ... CRAM
                        LOG_VDP("\tDMA transfer from %04x to CRAM @ %04x, length %04x, auto increment %04x\n", (v->dma_source_address_hi << 16 | v->dma_source_address_lo) << 1, v->access_address, v->dma_length, v->auto_increment);

                        do {
                            // "When doing a transfer to CRAM, the operation is aborted
                            //  once the address register is larger than 7Fh" - genvdp.txt
                            /*if (v->access_address > 0x7F)
                            break;*/

                            v->cram[v->access_address >> 1 & 0x3F] = m68k_read_w(v->cpu, (v->dma_source_address_hi << 16 | v->dma_source_address_lo) << 1);

                            ++v->dma_source_address_lo;
                            v->access_address += v->auto_increment;
                        } while (--v->dma_length);

                        break;

                    case 5:
                        LOG_VDP("\tDMA transfer from %04x to VSRAM @ %04x, length %04x, auto increment %04x\n", (v->dma_source_address_hi << 16 | v->dma_source_address_lo) << 1, v->access_address, v->dma_length, v->auto_increment);

                        do {
                            v->vsram[v->access_address >> 1 & 0x3F] = m68k_read_w(v->cpu, (v->dma_source_address_hi << 16 | v->dma_source_address_lo) << 1);

                            ++v->dma_source_address_lo;
                            v->access_address += v->auto_increment;
                        } while (--v->dma_length);

                        break;
                    }
                }
                // VRAM fill (will be triggered on the next data port write)
                else if (v->dma_type == 2)
                {
                    LOG_VDP("\tPending DMA fill\n");

                    v->pending_dma_fill = true;
                }
                // VRAM copy
                else if (v->dma_type == 3)
                {
                    LOG_VDP("WARNING! DMA copy not implemented\n");
                }
            }
        }
    }
}

uint16_t vdp_get_hv_counter(Vdp* v)
{
    return (v->beam_position_v << 8 & 0xFF00) | (v->beam_position_h >> 1 & 0xFF);
}

void draw_pattern(Vdp* v, int id, uint16_t* palette, int x, int y)
{
    uint16_t offset = id * 32;
    //LOG_VDP("drawing pattern %d at %d %d\n", id, x, y);
    for (int py = 0; py < 8; ++py)
        for (int px = 0; px < 4; ++px)
        {
            // 1 byte = 2 pixels
            uint8_t color_indexes = v->vram[offset + py * 4 + px];

            uint8_t color_index = (color_indexes & 0xF0) >> 4;
            if (color_index > 0) {
                uint16_t color = palette[color_index];
                SDL_SetRenderDrawColor(v->renderer, RED_8(color), GREEN_8(color), BLUE_8(color), 255);
                SDL_RenderDrawPoint(v->renderer, x + px * 2, y + py);
            }
            //printf("drawing pixels %d %d (%d) / %d %d (%d)\n", x + px * 2, y + py, color_index, x + px * 2 + 1, y + py, color_indexes & 0x0F);

            color_index = color_indexes & 0x0F;
            if (color_index > 0) {
                uint16_t color = palette[color_index];
                SDL_SetRenderDrawColor(v->renderer, RED_8(color), GREEN_8(color), BLUE_8(color), 255);
                SDL_RenderDrawPoint(v->renderer, x + px * 2 + 1, y + py);
            }
        }
}

void draw_plane(Vdp* v, int x, int y, uint8_t* nametable)
{
    for (int py = 0; py < v->vertical_plane_size; ++py)
        for (int px = 0; px < v->horizontal_plane_size; ++px)
        {
            uint16_t offset = py * v->horizontal_plane_size * 2 + px * 2;
            uint16_t data = (nametable[offset] << 8) | nametable[offset + 1];

            bool priority = BIT(data, 15);
            uint16_t* palette = v->cram + FRAGMENT(data, 14, 13) * 16;
            bool vertical_flip = BIT(data, 12);
            bool horizontal_flip = BIT(data, 11);
            uint16_t pattern = FRAGMENT(data, 10, 0);

            draw_pattern(v, pattern, palette, x + px * 8, y + py * 8);
        }
}

void vdp_draw_debug(Vdp* v)
{
    if (v->renderer == NULL)
        return;

    //SDL_SetRenderDrawColor(v->renderer, 0, 0, 0, 255);
    //SDL_RenderClear(v->renderer);
    SDL_SetRenderDrawColor(v->renderer, 0, 0, 0, 255);
    SDL_Rect debug_area = { 0, 0, 900, 900 };
    SDL_RenderFillRect(v->renderer, &debug_area);

    // Draw the color palette
    SDL_Rect cell = { 0, 0, 10, 10 };
    for (int row = 0; row < 4; ++row)
    {
        for (int col = 0; col < 16; ++col)
        {
            uint16_t color = v->cram[row * 16 + col];
            SDL_SetRenderDrawColor(v->renderer, RED_8(color), GREEN_8(color), BLUE_8(color), 255);
            SDL_RenderFillRect(v->renderer, &cell);

            cell.x += 10;
        }

        cell.x = 0;
        cell.y += 10;
    }

    // Draw patterns
    int columns = 30;
    for (int pattern = 0; pattern < 0x7FF; ++pattern)
        draw_pattern(v, pattern, debug_palette, 8 * (pattern % columns), 50 + 8 * (pattern / columns));

    // Draw the planes
    draw_plane(v, 300, 0, v->vram + v->plane_a_nametable);
    draw_plane(v, 300, 400, v->vram + v->plane_b_nametable);
    draw_plane(v, 300, 800, v->vram + v->window_nametable);
}

void vdp_draw_pattern_scanline(Vdp* v, uint16_t pattern_id, uint8_t line, uint16_t* palette, bool horizontal_flip, bool vertical_flip, int x, int y)
{
    // TODO flip
    // TODO render: directly write to bitmap?
    // TODO directly pass pattern pointer

    uint16_t pattern_offset = pattern_id * 32 + line * 4;

    for (int px = 0; px < 4; ++px)
    {
        // 1 byte = 2 pixels
        uint8_t color_indexes = v->vram[pattern_offset + px];

        uint8_t color_index = (color_indexes & 0xF0) >> 4;
        if (color_index > 0) {
            uint16_t color = palette[color_index];
            SDL_SetRenderDrawColor(v->renderer, RED_8(color), GREEN_8(color), BLUE_8(color), 255);
            SDL_RenderDrawPoint(v->renderer, x + px * 2, y);
        }

        color_index = color_indexes & 0x0F;
        if (color_index > 0) {
            uint16_t color = palette[color_index];
            SDL_SetRenderDrawColor(v->renderer, RED_8(color), GREEN_8(color), BLUE_8(color), 255);
            SDL_RenderDrawPoint(v->renderer, x + px * 2 + 1, y);
        }
    }
}

void vdp_draw_plane_scanline(Vdp* v, uint8_t* nametable, int line, int x, int y)
{
    // TODO scrolling

    uint16_t line_offset = (line / 8) * v->horizontal_plane_size * 2; // 1 nametable entry = 2 bytes

    for (int pattern_x = 0; pattern_x < v->horizontal_plane_size; ++pattern_x)
    {
        uint16_t pattern_offset = line_offset + pattern_x * 2;
        uint16_t pattern_data = (nametable[pattern_offset] << 8) | nametable[pattern_offset + 1];

        // Extract the pattern info - http://md.squee.co/VDP#Nametables
        bool priority = BIT(pattern_data, 15);
        uint16_t* palette = v->cram + FRAGMENT(pattern_data, 14, 13) * 16;
        bool vertical_flip = BIT(pattern_data, 12);
        bool horizontal_flip = BIT(pattern_data, 11);
        uint16_t id = FRAGMENT(pattern_data, 10, 0);

        uint16_t pattern_line = line % 8;

        vdp_draw_pattern_scanline(v, id, pattern_line, palette, horizontal_flip, vertical_flip, x + pattern_x * 8, y);
    }
}

bool vdp_get_plane_pixel_color(Vdp* v, uint8_t* nametable, int x, int y, uint16_t* color, bool* priority)
{
    // Get the pattern at the specified pixel coordinates

    uint16_t pattern_offset = (y / 8 * v->horizontal_plane_size + x / 8) * 2; // * 2 because one nametable entry is two bytes
    uint16_t pattern_data = (nametable[pattern_offset] << 8) | nametable[pattern_offset + 1];

    // Extract the pattern attributes
    // http://md.squee.co/VDP#Nametables

    uint16_t pattern_id = FRAGMENT(pattern_data, 10, 0);
    uint8_t palette_index = FRAGMENT(pattern_data, 14, 13);
    bool vertical_flip = BIT(pattern_data, 12);
    bool horizontal_flip = BIT(pattern_data, 11);
    *priority = BIT(pattern_data, 15);

    // TODO handle flipping
    // TODO handle scrolling
    uint8_t pattern_y = y % 8;
    uint8_t pattern_x = x % 8;

    // Get the pixel of the pattern at the specified position

    uint16_t pixel_offset = pattern_id * 32 + pattern_y * 4 + pattern_x / 2; // / 2 because one byte encodes two pixels

    uint8_t color_indexes = v->vram[pixel_offset];
    uint8_t color_index = pattern_x % 2 == 0 ? (color_indexes & 0xF0) >> 4 : color_indexes & 0x0F;

    // If the color is 0, this means transparent and nothing needs to be drawn
    if (color_index == 0)
        return false;

    *color = v->cram[palette_index * 16 + color_index];

    return true;
}

void vdp_draw_scanline(Vdp* v, int line)
{
    uint16_t background_color = v->cram[v->background_color_palette * 16 + v->background_color_entry];

    // Draw the scanline, pixel by pixel
    for (uint16_t pixel = 0; pixel < v->horizontal_plane_size * 8; ++pixel)
    {
        // Get the pixel values for each plane

        uint16_t plane_a_color, plane_b_color, sprites_color;
        bool plane_a_priority, plane_b_priority, sprites_priority;

        bool plane_a_drawn = vdp_get_plane_pixel_color(v, v->vram + v->plane_a_nametable, pixel, line, &plane_a_color, &plane_a_priority);
        bool plane_b_drawn = vdp_get_plane_pixel_color(v, v->vram + v->plane_b_nametable, pixel, line, &plane_b_color, &plane_b_priority);

        // Use the color from the plane with the highest priority

        uint16_t pixel_color;

        if (plane_a_drawn && plane_a_priority)
            pixel_color = plane_a_color;
        else if (plane_b_drawn && plane_b_priority)
            pixel_color = plane_b_color;
        else if (plane_a_drawn)
            pixel_color = plane_a_color;
        else if (plane_b_drawn)
            pixel_color = plane_b_color;
        else
            pixel_color = background_color;

        // Draw
        // TODO direct to bitmap
        SDL_SetRenderDrawColor(v->renderer, RED_8(pixel_color), GREEN_8(pixel_color), BLUE_8(pixel_color), 255);
        SDL_Rect draw_area = { 900 + pixel, line, 500, 1 };
        SDL_RenderFillRect(v->renderer, &draw_area);
    }

    //

    ++v->beam_position_v;

    if (v->beam_position_v % 20 == 0 && v->hblank_enabled)
        m68k_request_interrupt(v->cpu, HBLANK_IRQ);

    if (v->beam_position_v > 320)
    {
        v->beam_position_v = 0;

        if (v->vblank_enabled)
            m68k_request_interrupt(v->cpu, VBLANK_IRQ);

        // TODO tmp to debug faster
        vdp_draw_debug(v);

        SDL_RenderPresent(v->renderer);
    }
}
