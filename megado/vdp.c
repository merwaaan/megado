#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "genesis.h"
#include "m68k/m68k.h"
#include "vdp.h"

#ifdef DEBUG
#define LOG_VDP(...) printf(__VA_ARGS__)
#else
#define LOG_VDP(...)
#endif

// Display size values (register 0x1 and 0xC)
static uint8_t display_height_values[] = { 28, 30 };
static uint8_t display_width_values[] = { 32, 40 };

// Plane size values (register 0x10)
static uint8_t plane_size_values[] = { 32, 64, 0, 128 };

Vdp* vdp_make(Genesis* genesis)
{
    Vdp* v = calloc(1, sizeof(Vdp));
    v->genesis = genesis;

    v->output_buffer = calloc(BUFFER_SIZE, sizeof(uint8_t));

    v->pending_command = false;
    v->display_width = display_width_values[0];
    v->display_height = display_height_values[0];
    v->plane_width = plane_size_values[0];
    v->plane_height = plane_size_values[0];

    return v;
}

void vdp_free(Vdp* v)
{
    if (v == NULL)
        return;

    free(v->output_buffer);
    free(v);
}

void vdp_initialize(Vdp* v)
{
    memset(v->vram, 0, 0x10000 * sizeof(uint8_t));
    memset(v->vsram, 0, 0x40 * sizeof(uint16_t));
    memset(v->cram, 0, 0x40 * sizeof(Color));

    // Reset the internal state

    v->pending_command = false;
    v->pending_dma_fill = false;
    v->dma_in_progress = false;
    v->hblank_in_progress = false;
    v->vblank_in_progress = false;
    v->vblank_pending = false;

    v->h_counter = 0;
    v->v_counter = 0;
    v->hblank_counter = 0;
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

        value = COLOR_STRUCT_TO_11(v->cram[v->access_address >> 1 & 0x3F]);
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

        Color color = COLOR_11_TO_STRUCT(value);
        v->cram[v->access_address >> 1 & 0x3F] = color;
        v->access_address += v->auto_increment;
        break;

    case 5: // VSRAM write

        LOG_VDP("\tWrite %04x to VSRAM @ %02x\n", value, v->access_address >> 1);

        v->vsram[v->access_address >> 1 & 0x3F] = value;
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
                v->cram[v->access_address >> 1 & 0x3F] = COLOR_11_TO_STRUCT(value);
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
        (v->genesis->region == Region_Europe); // NTSC (0) / PAL (1)
}

void vdp_write_control(Vdp* v, uint16_t value)
{
    LOG_VDP("[%0x] control write: %02x\n", v->genesis->m68k->instruction_address, value);

    // TODO see https://sourceforge.net/p/dgen/dgen/ci/master/tree/vdp.cpp for cancelling commands

    // Register write
    if ((value & 0xC000) == 0x8000)
    {
        uint8_t reg = FRAGMENT(value, 12, 8);
        uint8_t reg_value = WORD_LO(value);

        if (reg > 0x17)
        {
            LOG_VDP("\t\tUnhandled register %02X\n", reg);
            return;
        }

        LOG_VDP("\tRegister %02X, value %02X\n", reg, reg_value);

        // Store the raw value
        v->register_raw_values[reg] = reg_value;

        // Update the internal state of the VDP
        switch (reg)
        {
        case 0:
            // TODO bit 5
            v->hblank_interrupt_enabled = BIT(reg_value, 4);
            v->hv_counter_latched = !BIT(reg_value, 1);
            // Bit 0 tells the VDP if the display is enabled or not. The difference with bit 6 of register 1 is unclear.

            LOG_VDP("\t\tH-blank interrupt %d, HV-counter latched %d\n", v->hblank_interrupt_enabled, v->hv_counter_latched);
            return;

        case 1:
            v->display_enabled = BIT(reg_value, 6);
            v->vblank_interrupt_enabled = BIT(reg_value, 5);
            v->dma_enabled = BIT(reg_value, 4);
            v->display_height = display_height_values[BIT(reg_value, 3)];

            LOG_VDP("\t\tDisplay enabled %d, V-blank interrupt %d, DMA enabled %d, Display mode %d\n", v->display_enabled, v->vblank_interrupt_enabled, v->dma_enabled, v->display_height);
            return;

        case 2:
            v->plane_a_nametable = (reg_value & 0x38) * 0x400;

            LOG_VDP("\t\tPlane A nametable %04x\n", v->plane_a_nametable);
            return;

        case 3:
            v->window_nametable = (reg_value & 0x3E) * 0x400;
            // TODO WD11 is ignored if the display resolution is 320px wide (H40), which limits the Window nametable address to multiples of $1000
            LOG_VDP("\t\tWindow nametable %04x\n", v->window_nametable);
            return;

        case 4:
            v->plane_b_nametable = ((reg_value & 7) << 3) * 0x400;

            LOG_VDP("\t\tPlane B nametable %04x\n", v->plane_b_nametable);
            return;

        case 5:
            v->sprites_attribute_table = FRAGMENT(reg_value, 6, 0) * 0x200;

            LOG_VDP("\t\tSprites attribute table %04x\n", v->sprites_attribute_table);
            return;

        case 7:
            v->background_color_palette = FRAGMENT(reg_value, 5, 4);
            v->background_color_entry = FRAGMENT(reg_value, 3, 0);

            LOG_VDP("\t\tBackground palette %d, entry %d\n", v->background_color_palette, v->background_color_entry);
            return;

        case 0xA:
            v->hblank_line = reg_value;

            LOG_VDP("\t\tH-blank counter %04x\n", v->hblank_line);
            return;

        case 0xB:
            v->vertical_scrolling_mode = BIT(reg_value, 2);
            v->horizontal_scrolling_mode = FRAGMENT(reg_value, 1, 0);

            LOG_VDP("\t\tVertical scrolling %d, Horizontal scrolling %d\n", v->vertical_scrolling_mode, v->horizontal_scrolling_mode);
            return;

        case 0xC:
            v->display_width = display_width_values[BIT(reg_value, 7)];
            v->shadow_highlight_enabled = BIT(reg_value, 3);
            v->interlace_mode = FRAGMENT(reg_value, 2, 1);

            LOG_VDP("\t\tDisplay width %d, Shadow/Highlight enabled %d, Interlace mode  %d\n", v->display_height, v->shadow_highlight_enabled, v->interlace_mode);
            if (v->interlace_mode != 0)
                printf("WARNING interlace mode not supported!");
            return;

        case 0xD:
            v->horizontal_scrolltable = FRAGMENT(reg_value, 5, 0) * 0x400;

            LOG_VDP("\t\tHorizontal scrolltable %d\n", v->horizontal_scrolltable);
            return;

        case 0xF:
            v->auto_increment = reg_value;

            LOG_VDP("\t\tAuto-increment %d\n", v->auto_increment);
            return;

        case 0x10:
            v->plane_width = plane_size_values[FRAGMENT(reg_value, 5, 4)];
            v->plane_height = plane_size_values[FRAGMENT(reg_value, 1, 0)];

            LOG_VDP("\t\tVertical plane size %d, Horizontal plane size %d\n", v->plane_width, v->plane_height);
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

        default:
            LOG_VDP("\t\tUnhandled register %02X\n", reg);
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
                            uint16_t value = m68k_read_w(v->genesis->m68k, (v->dma_source_address_hi << 16 | v->dma_source_address_lo) << 1);
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

                            uint16_t value = m68k_read_w(v->genesis->m68k, (v->dma_source_address_hi << 16 | v->dma_source_address_lo) << 1);
                            Color color = COLOR_11_TO_STRUCT(value);
                            v->cram[v->access_address >> 1 & 0x3F] = color;

                            ++v->dma_source_address_lo;
                            v->access_address += v->auto_increment;
                        } while (--v->dma_length);

                        break;

                    case 5:
                        LOG_VDP("\tDMA transfer from %04x to VSRAM @ %04x, length %04x, auto increment %04x\n", (v->dma_source_address_hi << 16 | v->dma_source_address_lo) << 1, v->access_address, v->dma_length, v->auto_increment);

                        do {
                            v->vsram[v->access_address >> 1 & 0x3F] = m68k_read_w(v->genesis->m68k, (v->dma_source_address_hi << 16 | v->dma_source_address_lo) << 1);

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
                    printf("WARNING! DMA copy not implemented\n");
                }
            }
        }
    }
}

uint16_t vdp_get_hv_counter(Vdp* v)
{
    return (v->v_counter << 8 & 0xFF00) | (v->h_counter >> 1 & 0xFF);
}

void vdp_get_resolution(Vdp* v, uint16_t* width, uint16_t* height)
{
    // The size of the display can be:
    //   - horizontally, 32 or 40 cell modes
    //   - vertically, 28 or 30 (restricted to PAL) cell modes

    *width = v->display_width * 8;
    *height = v->genesis->region == Region_Europe ? v->display_height * 8 : 28 * 8;
}

void vdp_get_plane_cell_data(Vdp* v, Planes plane, uint16_t cell_index, uint16_t* pattern_index, uint16_t* palette, bool* priority, bool* horizontal_flip, bool* vertical_flip)
{
    uint8_t* plane_offset = v->vram;
    switch (plane)
    {
    case Plane_A: plane_offset += v->plane_a_nametable; break;
    case Plane_B: plane_offset += v->plane_b_nametable; break;
    case Plane_Window: plane_offset += v->window_nametable; break;
    }

    uint16_t pattern_data = (plane_offset[cell_index * 2] << 8) | plane_offset[cell_index * 2 + 1];

    *priority = BIT(pattern_data, 15);
    *palette = FRAGMENT(pattern_data, 14, 13);
    *vertical_flip = BIT(pattern_data, 12);
    *horizontal_flip = BIT(pattern_data, 11);
    *pattern_index = FRAGMENT(pattern_data, 10, 0);
}

static Color color_black = { 0, 0, 0 };

void vdp_draw_pattern(Vdp* v, uint16_t pattern_index, Color* palette, uint8_t* buffer, uint32_t buffer_width, uint32_t x, uint32_t y, bool horizontal_flip, bool vertical_flip)
{
    uint16_t pattern_offset = pattern_index * 32;

    for (uint8_t py = 0; py < 8; ++py)
        for (uint8_t px = 0; px < 8; ++px)
        {
            // Handle flipping
            uint8_t flipped_px = horizontal_flip ? 7 - px : px;
            uint8_t flipped_py = vertical_flip ? 7 - py : py;

            uint32_t destination_offset = ((y + flipped_py) * buffer_width + x + flipped_px) * 3;

            // 1 byte holds the color data of 2 pixels
            uint8_t color_indexes = v->vram[pattern_offset + py * 4 + px / 2];

            uint8_t color_index = px % 2 == 0 ? (color_indexes & 0xF0) >> 4 : (color_indexes & 0x0F);
            if (color_index == 0)
                continue;

            Color color = color_index > 0 ? palette[color_index] : color_black;
            buffer[destination_offset] = color.r;
            buffer[destination_offset + 1] = color.g;
            buffer[destination_offset + 2] = color.b;
        }
}

void vdp_draw_plane(Vdp* v, Planes plane, uint8_t* buffer, uint32_t buffer_width)
{
    uint8_t* plane_offset = v->vram;
    switch (plane)
    {
    case Plane_A: plane_offset += v->plane_a_nametable; break;
    case Plane_B: plane_offset += v->plane_b_nametable; break;
    case Plane_Window: plane_offset += v->window_nametable; break;
    }

    uint16_t plane_width = plane == Plane_Window ? (v->display_width == 32 ? 32 : 64) : v->plane_height;
    uint16_t plane_height = plane == Plane_Window ? 32 : v->plane_width;

    for (int py = 0; py < plane_height; ++py)
        for (int px = 0; px < plane_width; ++px)
        {
            uint16_t pattern_offset = (py * plane_width + px) * 2;
            uint16_t pattern_data = (plane_offset[pattern_offset] << 8) | plane_offset[pattern_offset + 1];

            Color* palette = v->cram + FRAGMENT(pattern_data, 14, 13) * 16;
            bool vertical_flip = BIT(pattern_data, 12);
            bool horizontal_flip = BIT(pattern_data, 11);
            uint16_t pattern = FRAGMENT(pattern_data, 10, 0);

            vdp_draw_pattern(v, pattern, palette, buffer, buffer_width, px * 8, py * 8, horizontal_flip, vertical_flip);
        }
}

static void draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t* buffer, uint32_t buffer_width)
{
    // Top border
    uint32_t destination = (y * buffer_width + x) * 3;
    for (uint16_t px = x; px < x + w; ++px)
    {
        destination += 3;
        buffer[destination] = 0xFF;
        buffer[destination + 1] = 0xFF;
        buffer[destination + 2] = 0xFF;
    }

    // Left & right borders
    for (uint16_t py = y; py < y + h; ++py)
    {
        destination = (py * buffer_width + x) * 3;
        buffer[destination] = 0xFF;
        buffer[destination + 1] = 0xFF;
        buffer[destination + 2] = 0xFF;
        destination += w * 3;
        buffer[destination] = 0xFF;
        buffer[destination + 1] = 0xFF;
        buffer[destination + 2] = 0xFF;
    }

    // Bottom borders
    destination = ((y + h) * buffer_width + x) * 3;
    for (uint16_t px = x; px < x + w; ++px)
    {
        destination += 3;
        buffer[destination] = 0xFF;
        buffer[destination + 1] = 0xFF;
        buffer[destination + 2] = 0xFF;
    }
}

void vdp_draw_sprites(Vdp* v, uint8_t* buffer, uint32_t buffer_width)
{
    uint8_t* attribute_table = v->vram + v->sprites_attribute_table;

    uint8_t sprite = 0;
    uint8_t sprite_counter = 0;
    do
    {
        uint8_t* attributes = attribute_table + sprite * 8;

        uint16_t x = ((attributes[6] & 1) << 8 | attributes[7]);
        uint16_t y = ((attributes[0] & 3) << 8 | attributes[1]);
        uint8_t width = FRAGMENT(attributes[2], 3, 2) + 1;
        uint8_t height = FRAGMENT(attributes[2], 1, 0) + 1;
        uint16_t pattern_index = (attributes[4] & 7) << 8 | attributes[5];
        uint8_t palette_index = FRAGMENT(attributes[4], 6, 5);
        bool vertical_flip = BIT(attributes[4], 4);
        bool horizontal_flip = BIT(attributes[4], 3);
        uint8_t link = attributes[3] & 0x7F;

        for (uint8_t py = 0; py < height; ++py)
            for (uint8_t px = 0; px < width; ++px)
            {
                uint16_t subpattern_index = pattern_index + px * height + py;
                uint16_t subpattern_x = horizontal_flip ? x + (width - 1 - px) * 8 : x + px * 8;
                uint16_t subpattern_y = vertical_flip ? y + (height - 1 - py) * 8 : y + py * 8;

                vdp_draw_pattern(v, subpattern_index, &v->cram[palette_index * 16], buffer, buffer_width, subpattern_x, subpattern_y, horizontal_flip, vertical_flip);
            }

        // Draw a border around the sprite
        draw_rect(x, y, width * 8, height * 8, buffer, buffer_width);

        ++sprite_counter;

        // Move on to the next sprite
        sprite = link;

    } while (sprite != 0 && sprite_counter < 64);
}

typedef struct {
    bool drawn[BUFFER_WIDTH];
    Color colors[BUFFER_WIDTH];
    bool priorities[BUFFER_WIDTH];
} ScanlineData;

static ScanlineData plane_a_scanline;
static ScanlineData plane_b_scanline;
static ScanlineData plane_w_scanline;
static ScanlineData sprites_scanline;

void vdp_get_plane_scanline(Vdp* v, Planes plane, int scanline, ScanlineData* data)
{
    // Exit early if we are rendering the window plane but it is not visible on that scanline.
    //
    // This can occur due to the following circumstances:
    // - The window plane is disabled (reg 0x11 is 0 && reg 0x12 is 0)
    // - The window plane is visible from the first line to the offset (direction is 0) but the scanline is below
    // - The window plane is visible from the offset to the last line (direction is 1) but the scanline is above
    //
    // https://emudocs.org/Genesis/Graphics/genvdp.txt
    // http://gendev.spritesmind.net/forum/viewtopic.php?f=2&t=2492&p=30175#p30183

    if (plane == Plane_Window && (
        (v->register_raw_values[0x11] == 0 && v->register_raw_values[0x12] == 0) || // The window is disabled
        (scanline >= v->window_plane_vertical_offset * 8) ^ v->window_plane_vertical_direction)) // The window is not visible on that line
    {
        for (uint16_t i = 0; i < BUFFER_WIDTH; ++i)
            data->drawn[i] = false;
        return;
    }

    uint8_t* plane_offset = v->vram;
    switch (plane)
    {
    case Plane_A: plane_offset += v->plane_a_nametable; break;
    case Plane_B: plane_offset += v->plane_b_nametable; break;
    case Plane_Window: plane_offset += v->window_nametable; break;
    }

    // Handle horizontal scrolling

    uint16_t horizontal_scroll = 0;

    if (plane != Plane_Window)
    {
        uint8_t* horizontal_scroll_offset = v->vram + v->horizontal_scrolltable;

        if (v->horizontal_scrolling_mode == HorizontalScrollingMode_Screen)
            horizontal_scroll_offset += plane == Plane_A ? 0 : 2;
        else if (v->horizontal_scrolling_mode == HorizontalScrollingMode_Row)
            horizontal_scroll_offset += scanline / 8 * 32 + (plane == Plane_A ? 0 : 2); // TODO use y before or after vertical scrolling?!
        else if (v->horizontal_scrolling_mode == HorizontalScrollingMode_Line)
            horizontal_scroll_offset += scanline * 4 + (plane == Plane_A ? 0 : 2); // TODO use y before or after vertical scrolling?!

        horizontal_scroll = (horizontal_scroll_offset[0] << 8 | horizontal_scroll_offset[1]) & 0x3FF;
    }

    // The size of planes A and B is defined by register 0x10.
    //
    // For the window plane:
    // - in H32 mode, it is 32 cells wide.
    // - in H40 mode, it is 64 cells wide.
    // - it seems to always be 32 cells high.
    // TODO any doc to confirm that?
    uint8_t plane_width = plane == Plane_Window ? (v->display_width == 32 ? 32 : 64) : v->plane_height;
    uint8_t plane_height = plane == Plane_Window ? 32 : v->plane_width;

    uint16_t screen_width = v->display_width * 8;
    for (uint16_t pixel = 0; pixel < screen_width; ++pixel)
    {
        // TODO horizontal windowing

        // Handle vertical scrolling

        uint16_t vertical_scroll = 0;

        if (plane != Plane_Window)
        {
            if (v->vertical_scrolling_mode == VerticalScrollingMode_Screen)
                vertical_scroll = v->vsram[plane == Plane_A ? 0 : 1];
            else if (v->vertical_scrolling_mode == VerticalScrollingMode_TwoColumns)
                vertical_scroll = v->vsram[pixel / 16 * 2 + (plane == Plane_A ? 0 : 1)] & 0x3FF; // TODO use x before or after horizontal scrolling?!
        }

        uint16_t x = (uint16_t)(pixel - horizontal_scroll) % (plane_width * 8);
        uint16_t y = (uint16_t)(scanline + vertical_scroll) % (plane_height * 8);

        // Get the pattern at the specified pixel coordinates
        uint16_t pattern_offset = (y / 8 * plane_width + x / 8) * 2; // * 2 because one nametable entry is two bytes
        uint16_t pattern_data = (plane_offset[pattern_offset] << 8) | plane_offset[pattern_offset + 1];

        // Extract the pattern attributes
        // http://md.squee.co/VDP#Nametables
        // TODO do this once per pattern, not once per pixel!!

        uint16_t pattern_index = FRAGMENT(pattern_data, 10, 0);
        uint8_t palette_index = FRAGMENT(pattern_data, 14, 13);
        bool vertical_flip = BIT(pattern_data, 12);
        bool horizontal_flip = BIT(pattern_data, 11);
        data->priorities[pixel] = BIT(pattern_data, 15);

        uint8_t pattern_y = vertical_flip ? 7 - y % 8 : y % 8;
        uint8_t pattern_x = horizontal_flip ? 7 - x % 8 : x % 8;

        // Get the pixel of the pattern at the specified position

        uint16_t pixel_offset = pattern_index * 32 + pattern_y * 4 + pattern_x / 2; // / 2 because one byte encodes two pixels

        uint8_t color_indexes = v->vram[pixel_offset];
        uint8_t color_index = pattern_x % 2 == 0 ? (color_indexes & 0xF0) >> 4 : color_indexes & 0x0F;

        // Zero means transparent
        if (color_index == 0)
        {
            data->drawn[pixel] = false;
            continue;
        }

        data->colors[pixel] = v->cram[palette_index * 16 + color_index];
        data->drawn[pixel] = true;
    }
}

void vdp_get_sprites_scanline(Vdp* v, int scanline, ScanlineData* data)
{
    uint8_t* attribute_table = v->vram + v->sprites_attribute_table;

    // Clear the scanline
    for (uint16_t i = 0; i < BUFFER_WIDTH; ++i)
        data->drawn[i] = false;

    uint8_t sprite = 0;
    uint8_t sprite_counter = 0;
    do
    {
        uint8_t* attributes = attribute_table + sprite * 8;

        // Extract the sprite attributes
        // http://md.squee.co/VDP#Sprite_Attribute_Table

        uint16_t x = ((attributes[6] & 1) << 8 | attributes[7]) - 128; // Coordinates in screen-space
        uint16_t y = ((attributes[0] & 3) << 8 | attributes[1]) - 128;

        uint8_t width = FRAGMENT(attributes[2], 3, 2) + 1;
        uint8_t height = FRAGMENT(attributes[2], 1, 0) + 1;

        uint16_t pattern_index = (attributes[4] & 7) << 8 | attributes[5];
        uint8_t palette_index = FRAGMENT(attributes[4], 6, 5);

        bool vertical_flip = BIT(attributes[4], 4);
        bool horizontal_flip = BIT(attributes[4], 3);
        bool priority = BIT(attributes[4], 7);

        uint8_t link = attributes[3] & 0x7F;

        // Render sprites that appear on the scanline

        uint8_t total_height = height * 8;

        if (scanline >= y && scanline < y + total_height)
        {
            uint8_t sprite_y = vertical_flip ? total_height - (scanline - y) - 1 : scanline - y;

            uint8_t total_width = width * 8;

            for (uint8_t sprite_x = 0; sprite_x < total_width; ++sprite_x)
            {
                int16_t scanline_x = x + (horizontal_flip ? total_width - sprite_x - 1 : sprite_x);

                if (scanline_x < 0)
                    continue;

                // TODO: proper right bound
                // for now, it certainly cannot be larger than BUFFER_WIDTH,
                // otherwise it might corrupt memory
                if (scanline_x >= BUFFER_WIDTH)
                    break;

                // Sprites are drawn front-to-back so don't draw over previous sprites
                if (data->drawn[scanline_x])
                    continue;

                uint16_t column_offset = sprite_x / 8 * height * 32;

                uint16_t pixel_offset = pattern_index * 32 + column_offset + sprite_y * 4 + sprite_x % 8 / 2;
                uint8_t color_indexes = v->vram[pixel_offset];
                uint8_t color_index = sprite_x % 2 == 0 ? (color_indexes & 0xF0) >> 4 : color_indexes & 0x0F;

                // Zero means transparent
                if (color_index == 0)
                {
                    data->drawn[scanline_x] = false;
                    continue;
                }

                data->drawn[scanline_x] = true;
                data->colors[scanline_x] = v->cram[palette_index * 16 + color_index];
                data->priorities[scanline_x] = priority;
            }
        }

        ++sprite_counter;

        // Move on to the next sprite
        sprite = link;

        // TODO Sprite masking https://emudocs.org/Genesis/Graphics/genvdp.txt

    } while (sprite != 0 // 0 means the end of the linked list
        && sprite_counter < 64 // exit the loop in case of bad linked lists
        // At most 16 / 20 sprites can be displayed by line
        // disabled for now
        /* (v->display_width == 32 && sprite_counter <= 16 */
        /*  || v->display_width == 40 && sprite_counter <= 20) */
        );
}

void render_scanline(Vdp* v, int scanline)
{
    Color background_color = v->cram[v->background_color_palette * 16 + v->background_color_entry];

    // Get color & priority data for each layer
    vdp_get_plane_scanline(v, Plane_A, scanline, &plane_a_scanline);
    vdp_get_plane_scanline(v, Plane_B, scanline, &plane_b_scanline);
    vdp_get_plane_scanline(v, Plane_Window, scanline, &plane_w_scanline);
    vdp_get_sprites_scanline(v, scanline, &sprites_scanline);

    // Combine the layers
    uint16_t screen_width = v->display_width * 8;
    for (uint16_t pixel = 0; pixel < screen_width; ++pixel)
    {
        // Use the color from the layer with the highest priority
        // TODO more details

        Color pixel_color;

        if (plane_w_scanline.drawn[pixel] && plane_w_scanline.priorities[pixel]) // Window *
            pixel_color = plane_w_scanline.colors[pixel];
        else if (sprites_scanline.drawn[pixel] && sprites_scanline.priorities[pixel]) // Sprites *
            pixel_color = sprites_scanline.colors[pixel];
        else if (plane_a_scanline.drawn[pixel] && plane_a_scanline.priorities[pixel]) // A *
            pixel_color = plane_a_scanline.colors[pixel];
        else if (plane_b_scanline.drawn[pixel] && plane_b_scanline.priorities[pixel]) // B *
            pixel_color = plane_b_scanline.colors[pixel];
        else if (plane_w_scanline.drawn[pixel]) // Window
            pixel_color = plane_w_scanline.colors[pixel];
        else if (sprites_scanline.drawn[pixel]) // Sprites
            pixel_color = sprites_scanline.colors[pixel];
        else if (plane_a_scanline.drawn[pixel]) // A
            pixel_color = plane_a_scanline.colors[pixel];
        else if (plane_b_scanline.drawn[pixel]) // B
            pixel_color = plane_b_scanline.colors[pixel];
        else // Background
            pixel_color = background_color;

        uint32_t pixel_offset = (scanline * BUFFER_WIDTH + pixel) * 3;
        v->output_buffer[pixel_offset] = pixel_color.r;
        v->output_buffer[pixel_offset + 1] = pixel_color.g;
        v->output_buffer[pixel_offset + 2] = pixel_color.b;
    }
}

void vdp_draw_scanline(Vdp* v, int scanline)
{
    uint16_t output_width, output_height;
    vdp_get_resolution(v, &output_width, &output_height);

    if (v->display_enabled && scanline < output_height)
        render_scanline(v, scanline);

    /*
     * Handle horizontal interrupts
     *
     * - A line interrupt counter is decremented on each scanline
     * - An interrupt occurs if the counter reached 0 and horizontal
     *   interrupts are enabled
     * - The counter is reset to the value stored in register 0xA when
     *   an interrupt occurs OR on line 0 OR on lines >224
     */

     // Reload the counter
    if (scanline == 0 || scanline >= 225)// TODO PAL
        v->hblank_counter = v->hblank_line;

    // Trigger an interrupt when the counter reaches 0
    if (v->hblank_counter <= 0)
    {
        if (v->hblank_interrupt_enabled)
            m68k_request_interrupt(v->genesis->m68k, HBLANK_IRQ);

        v->hblank_counter = v->hblank_line;
    }

    --v->hblank_counter;

    /*
     * Handle vertical interrupts
     */

    v->v_counter = scanline;

    // V-blank occurs on line 224
    if (scanline == 224)// TODO PAL
    {
        v->vblank_in_progress = true;

        if (v->vblank_interrupt_enabled)
            m68k_request_interrupt(v->genesis->m68k, VBLANK_IRQ);
    }
    // V-blank ends on line 262
    else if (scanline == 261)// TODO PAL
    {
        v->vblank_in_progress = false;
    }
}

void vdp_draw_screen(Vdp* v)
{
    uint16_t output_width, output_height;
    vdp_get_resolution(v, &output_width, &output_height);

    if (v->display_enabled)
    for (int line = 0; line <= output_height; ++line)
        render_scanline(v, line);
}
