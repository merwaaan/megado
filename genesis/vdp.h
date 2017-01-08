#pragma once

#include <m68k/bit_utils.h>
#include <SDL.h>
#include <stdbool.h>
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

    // http://md.squee.co/VDP#VDP_Registers

    // Register $00
    bool skip_leftmost_pixels;
    bool hblank_enabled;
    bool latch_hv_counter;
    bool enabled;

    // Register $01
    bool vram_extended;
    bool display_enabled;
    bool vblank_enabled;
    bool dma_enabled;
    bool display_mode;
    bool genesis_mode;

    // Register $03
    bool window_nametable;

    // Register $04
    bool plane_b_nametable;

    // Register $05
    bool sprites_location;

    // Register $06
    // TODO 128k mode ???

    // Register $07
    bool background_color_palette;
    bool background_color_entry;

    // Register $0A
    bool hblank_counter;

    // Register $0B
    // TODO light gun ???
    bool vertical_scrolling;
    bool horizontal_scrolling;

    // Register $0C
    int display_width;
    bool shadow_highlight_enabled;
    int interlace;
    // TODO external color data ???
    // TODO pixel clock ???

    // Register $0D
    int horizontal_scrolltable;

    // Register $0E
    // TODO 128k mode ???

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
    int dma_length;

    // Register $15 - $17
    int dma_source;
    int dma_type;

    SDL_Window* window;
    SDL_Renderer* renderer;
} Vdp;

Vdp* vdp_make();
void vdp_free(Vdp*);

uint8_t vdp_data_read();
void vdp_data_write();

uint8_t vdp_control_read();
void vdp_control_write();

void vdp_draw(Vdp*);
