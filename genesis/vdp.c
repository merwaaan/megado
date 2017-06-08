#include <assert.h>
#include <m68k/m68k.h>
#include <stdlib.h>
#include <stdio.h>

#include "vdp.h"

#undef DEBUG

#ifdef DEBUG
#define LOG_VDP(...) printf(__VA_ARGS__)
#else
#define LOG_VDP(...)
#endif

void draw(Vdp* v);

// Display width values (register 0xC)
uint8_t display_width_values[] = { 32, 40 };

// Plane size values (register 0x10)
uint8_t plane_size_values[] = { 32, 64, 0, 128 };

Vdp* vdp_make(M68k* cpu)
{
    Vdp* v = calloc(1, sizeof(Vdp));
    v->cpu = cpu;
    v->pending_command = false;

    v->vram = calloc(0x10000, sizeof(uint8_t));
    v->vsram = calloc(64, sizeof(uint16_t));
    v->cram = calloc(64, sizeof(Color));

    v->buffer = calloc(BUFFER_LENGTH, sizeof(uint8_t));

    return v;
}

void vdp_free(Vdp* v)
{
    if (v == NULL)
        return;

    free(v->vram);
    free(v->vsram);
    free(v->cram);
    free(v->buffer);
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
            // TODO bit 5
            v->hblank_enabled = BIT(reg_value, 4);
            v->hv_counter_latched = !BIT(reg_value, 1);
            // Bit 0 tells the VDP if the display is enabled or not. The difference with bit 6 of register 1 is unclear.

            LOG_VDP("\t\tH-blank enabled %d, HV-counter latched %d\n", v->hblank_enabled, v->hv_counter_latched);
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

            LOG_VDP("\t\tDisplay width %d, Shadow/Highlight enabled %d, Interlace mode  %d\n", v->display_mode, v->shadow_highlight_enabled, v->interlace_mode);
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
            v->vertical_plane_size = plane_size_values[FRAGMENT(reg_value, 5, 4)];
            v->horizontal_plane_size = plane_size_values[FRAGMENT(reg_value, 1, 0)];

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

                            uint16_t value = m68k_read_w(v->cpu, (v->dma_source_address_hi << 16 | v->dma_source_address_lo) << 1);
                            Color color = COLOR_11_TO_STRUCT(value); 
                            v->cram[v->access_address >> 1 & 0x3F] = color;

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
    return (v->v_counter << 8 & 0xFF00) | (v->h_counter >> 1 & 0xFF);
}

static Color color_black = { 0, 0, 0 };

void vdp_draw_pattern(Vdp* v, uint16_t pattern_index, Color* palette, uint8_t* buffer, uint32_t buffer_width, uint32_t x, uint32_t y)
{
    uint16_t pattern_offset = pattern_index * 32;

    for (uint8_t pixel_pair = 0; pixel_pair < 32; ++pixel_pair)
    {
        uint32_t pixel_offset = ((y + pixel_pair / 4) * buffer_width + x + pixel_pair % 4 * 2) * 3;

        // 1 byte holds the color data of 2 pixels
        uint8_t color_indexes = v->vram[pattern_offset + pixel_pair];

        uint8_t color_index = (color_indexes & 0xF0) >> 4;
        Color color = color_index > 0 ? palette[color_index] : color_black;
        buffer[pixel_offset] = color.r;
        buffer[pixel_offset + 1] = color.g;
        buffer[pixel_offset + 2] = color.b;

        color_index = color_indexes & 0x0F;
        color = color_index > 0 ? palette[color_index] : color_black;
        buffer[pixel_offset + 3] = color.r;
        buffer[pixel_offset + 4] = color.g;
        buffer[pixel_offset + 5] = color.b;
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

    for (int py = 0; py < v->horizontal_plane_size; ++py)
        for (int px = 0; px < v->horizontal_plane_size; ++px)
        {
            uint16_t pattern_offset = (py * v->horizontal_plane_size + px) * 2;
            uint16_t data = (plane_offset[pattern_offset] << 8) | plane_offset[pattern_offset + 1];

            Color* palette = v->cram + FRAGMENT(data, 14, 13) * 16;
            bool vertical_flip = BIT(data, 12);
            bool horizontal_flip = BIT(data, 11);
            uint16_t pattern = FRAGMENT(data, 10, 0);

            vdp_draw_pattern(v, pattern, palette, buffer, buffer_width, px * 8, py * 8);
        }
}

// TODO clean this up
typedef struct {
    bool drawn[312]; // TODO how many?
    Color colors[312];
    bool priorities[312];
} ScanlineData;

ScanlineData plane_a_scanline;
ScanlineData plane_b_scanline;
ScanlineData plane_w_scanline;
ScanlineData sprites_scanline;

void vdp_get_plane_scanline(Vdp* v, Planes plane, int scanline, ScanlineData* data)
{
    uint8_t* name_table = v->vram + (plane == Plane_A ? v->plane_a_nametable : v->plane_b_nametable);
    // TODO window

    // Handle horizontal scrolling

    uint8_t* horizontal_scroll_offset = v->vram + v->horizontal_scrolltable;

    if (v->horizontal_scrolling_mode == HorizontalScrollingMode_Screen)
        horizontal_scroll_offset += plane == Plane_A ? 0 : 2;
    else if (v->horizontal_scrolling_mode == HorizontalScrollingMode_Row)
        horizontal_scroll_offset += scanline / 8 * 32 + (plane == Plane_A ? 0 : 2); // TODO use y before or after vertical scrolling?!
    else if (v->horizontal_scrolling_mode == HorizontalScrollingMode_Line)
        horizontal_scroll_offset += scanline * 4 + (plane == Plane_A ? 0 : 2); // TODO use y before or after vertical scrolling?!

    uint16_t horizontal_scroll = (horizontal_scroll_offset[0] << 8 | horizontal_scroll_offset[1]) & 0x3FF;

    uint16_t screen_width = v->display_width * 8;
    for (uint16_t pixel = 0; pixel < screen_width; ++pixel)
    {
        // Handle vertical scrolling

        uint16_t vertical_scroll = 0;

        if (v->vertical_scrolling_mode == VerticalScrollingMode_Screen)
            vertical_scroll = v->vsram[plane == Plane_A ? 0 : 1];
        else if (v->vertical_scrolling_mode == VerticalScrollingMode_TwoColumns)
            vertical_scroll = v->vsram[pixel / 16 * 2 + (plane == Plane_A ? 0 : 1)] & 0x3FF; // TODO use x before or after horizontal scrolling?!

        uint16_t x = (uint16_t)(pixel - horizontal_scroll) % (v->horizontal_plane_size * 8);
        uint16_t y = (uint16_t)(scanline + vertical_scroll) % (v->vertical_plane_size * 8);

        // Get the pattern at the specified pixel coordinates
        uint16_t pattern_offset = (y / 8 * v->horizontal_plane_size + x / 8) * 2; // * 2 because one nametable entry is two bytes
        uint16_t pattern_data = (name_table[pattern_offset] << 8) | name_table[pattern_offset + 1];

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

    uint8_t sprite = 0;
    do
    {
        uint8_t* attributes = attribute_table + sprite * 8;

        // Extract the sprite attributes
        // http://md.squee.co/VDP#Sprite_Attribute_Table

        int16_t x = ((attributes[6] & 1) << 8 | attributes[7]) - 128; // Coordinates in screen-space
        int16_t y = ((attributes[0] & 3) << 8 | attributes[1]) - 128;

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
            uint8_t sprite_y = scanline - y; // TODO handle flipping

            uint8_t total_width = width * 8;

            for (uint8_t sprite_x = 0; sprite_x < total_width; ++sprite_x)
            {
                int16_t scanline_x = x + (horizontal_flip ? total_width - sprite_x - 1: sprite_x);

                if (scanline_x < 0) // TODO right bound
                    continue;

                uint16_t column_offset = sprite_x / 8 * height * 32;

                // TODO group pixel pairs
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

        // Move on to the next sprite
        sprite = link;

        // TODO sizes
        // TODO priority between sprites? (or link-order dependent?)
        // TODO Sprite at x=0 mask low priority sprites or something
    } while (sprite != 0);
    // TODO sprite count limit
}

void vdp_draw_scanline(Vdp* v, int scanline)
{
    if (v->display_enabled && v->v_counter < 224) // TODO PAL
    {
        Color background_color = v->cram[v->background_color_palette * 16 + v->background_color_entry];

        // Get pixel & priority data for each layer
        vdp_get_plane_scanline(v, Plane_A, scanline, &plane_a_scanline);
        vdp_get_plane_scanline(v, Plane_B, scanline, &plane_b_scanline);
        // TODO vdp_get_plane_scanline(v, Plane_W, line, &plane_w_scanline);
        vdp_get_sprites_scanline(v, scanline, &plane_a_scanline);

        // Combine the layers
        uint16_t screen_width = v->display_width * 8;
        for (uint16_t pixel = 0; pixel < screen_width; ++pixel)
        {
            // Use the color from the plane with the highest priority
            // TODO more details

            Color pixel_color;

            // meh, could do better?
            if (sprites_scanline.drawn[pixel] && sprites_scanline.priorities[pixel])
                pixel_color = sprites_scanline.colors[pixel];
            else if (plane_a_scanline.drawn[pixel] && plane_a_scanline.priorities[pixel])
                pixel_color = plane_a_scanline.colors[pixel];
            else if (plane_b_scanline.drawn[pixel] && plane_b_scanline.priorities[pixel])
                pixel_color = plane_b_scanline.colors[pixel];
            else if (sprites_scanline.drawn[pixel])
                pixel_color = sprites_scanline.colors[pixel];
            else if (plane_a_scanline.drawn[pixel])
                pixel_color = plane_a_scanline.colors[pixel];
            else if (plane_b_scanline.drawn[pixel])
                pixel_color = plane_b_scanline.colors[pixel];
            else
                pixel_color = background_color;

            // TODO put pixel func
            // TODO as uint32?
            uint32_t pixel_offset = (scanline * BUFFER_WIDTH + pixel) * 3;
            v->buffer[pixel_offset] = pixel_color.r;
            v->buffer[pixel_offset + 1] = pixel_color.g;
            v->buffer[pixel_offset + 2] = pixel_color.b;

            // TODO direct to bitmap
            /*SDL_SetRenderDrawColor(v->renderer, RED_8(pixel_color), GREEN_8(pixel_color), BLUE_8(pixel_color), 255);
            SDL_Rect draw_area = { 900 + pixel, line, 1, 1 };
            SDL_RenderFillRect(v->renderer, &draw_area);*/
        }
    }

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
    if (v->v_counter == 0 || v->v_counter >= 225)// TODO PAL
        v->hblank_counter = v->hblank_line;

    // Trigger an interrupt when the counter reaches 0
    if (v->hblank_counter <= 0)
    {
        if (v->hblank_enabled)
            m68k_request_interrupt(v->cpu, HBLANK_IRQ);

        v->hblank_counter = v->hblank_line;
    }

    --v->hblank_counter;

    /*
     * Handle vertical interrupts
     */

    ++v->v_counter;

    // V-blank occurs on line 224
    if (v->v_counter == 224 && v->vblank_enabled)// TODO PAL
    {
        v->vblank_in_progress = true;

        m68k_request_interrupt(v->cpu, VBLANK_IRQ);
    }
    // V-blank ends on line 262
    else if (v->v_counter == 262)// TODO PAL
    {
        v->vblank_in_progress = false;
        v->v_counter = 0;

        // TODO tmp to debug faster
        //vdp_draw_debug(v);
    }
}
