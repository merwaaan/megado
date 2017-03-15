#pragma once

#include <m68k/bit_utils.h>
#include <SDL.h>
#include <stdbool.h>
#include <stdint.h>

#define HBLANK_IRQ 4
#define VBLANK_IRQ 6

// Extract color components from a palette word
#define RED(c) FRAGMENT((c), 3, 1)
#define GREEN(c) FRAGMENT((c), 7, 5)
#define BLUE(c) FRAGMENT((c), 11, 9)

// Convert 3-bit components to 8-bit
#define COLOR_8(c) (c) * 32
#define RED_8(c) COLOR_8(RED(c))
#define GREEN_8(c) COLOR_8(GREEN(c))
#define BLUE_8(c) COLOR_8(BLUE(c))

typedef struct Vdp
{
    uint8_t* vram;
    uint16_t* cram;
    uint16_t* vsram;

    // If set, a first command word has been written.
    // We're waiting for the second half.
    bool pending_command;

    // Read/Write configuration.
    // Set through the control port.
    int access_mode;
    uint16_t access_address;

    // The next word written to the data port will trigger a DMA fill
    bool pending_dma_fill;

    // http://md.squee.co/VDP#VDP_Registers

    // Register $00
    bool hblank_enabled;
    bool hv_counter_enabled;

    // Register $01
    bool display_enabled;
    bool vblank_enabled;
    bool dma_enabled;
    bool display_mode;

    // Register $02
    uint32_t plane_a_nametable;

    // Register $03
    uint32_t window_nametable;

    // Register $04
    uint32_t plane_b_nametable;

    // Register $05
    uint32_t sprites_attributetable;

    // Register $07
    uint8_t background_color_palette;
    uint8_t background_color_entry;

    // Register $0A
    uint8_t hblank_counter; // TODO unclear

    // Register $0B
    uint8_t vertical_scrolling;
    uint8_t horizontal_scrolling;

    // Register $0C
    int display_width;
    bool shadow_highlight_enabled;
    int interlace_mode;

    // Register $0D
    int horizontal_scrolltable;

    // Register $0F
    int auto_increment;

    // Register $10
    int vertical_plane_size;
    int horizontal_plane_size;

    // Register $11
    bool window_plane_horizontal_direction;
    int window_plane_horizontal_offset;

    // Register $12
    bool window_plane_vertical_direction;
    int window_plane_vertical_offset;

    // Register $13 - $14
    uint16_t dma_length;

    // Register $15 - $17
    uint16_t dma_source_address_lo;
    uint8_t dma_source_address_hi;
    int dma_type;

    bool dma_in_progress;
    bool hblank_in_progress;
    bool vblank_in_progress;

    M68k* cpu;

    SDL_Window* window;
    SDL_Renderer* renderer;
} Vdp;

Vdp* vdp_make(M68k* cpu);
void vdp_free(Vdp*);

uint8_t vdp_read_data_hi(Vdp*);
uint8_t vdp_read_data_lo(Vdp*);
void vdp_write_data(Vdp*, uint16_t value); // TODO 8bit writes OK but only 16bit reads?
                                           // If confirmed, just use a single read_xxx, and return nibbles in m68k_io

uint16_t vdp_read_control(Vdp*);
void vdp_write_control(Vdp*, uint16_t value);

void vdp_draw(Vdp*);
