#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "m68k/bit_utils.h"

#define HBLANK_IRQ 4
#define VBLANK_IRQ 6

// Extract color components from a 11-bit color word
#define RED_COMPONENT_11(c) FRAGMENT((c), 3, 1)
#define GREEN_COMPONENT_11(c) FRAGMENT((c), 7, 5)
#define BLUE_COMPONENT_11(c) FRAGMENT((c), 11, 9)

// Convert 3-bit color components to 8-bit
#define COLOR_COMPONENT_3_TO_8(c) (c) * 32

// Convert a 11-bit color word to a color struct
#define COLOR_11_TO_STRUCT(c) (Color) { COLOR_COMPONENT_3_TO_8(RED_COMPONENT_11(c)), COLOR_COMPONENT_3_TO_8(GREEN_COMPONENT_11(c)), COLOR_COMPONENT_3_TO_8(BLUE_COMPONENT_11(c)) }

// Convert a color struct to a 11-bit color word
#define COLOR_STRUCT_TO_11(c) ((c.r / 32) << 1 | (c.g / 32) << 5 | (c.b / 32) << 9)

// The screen buffer has the same dimensions as the maximal resolution
#define BUFFER_WIDTH 320
#define BUFFER_HEIGHT 240
#define BUFFER_SIZE (BUFFER_WIDTH * BUFFER_HEIGHT * 3)

struct Genesis;

typedef enum Planes
{
    Plane_A,
    Plane_B,
    Plane_Window
} Planes;

typedef enum
{
    HorizontalScrollingMode_Screen,
    HorizontalScrollingMode_Invalid,
    HorizontalScrollingMode_Row,
    HorizontalScrollingMode_Line
} HorizontalScrollingModes;

typedef enum
{
    VerticalScrollingMode_Screen,
    VerticalScrollingMode_TwoColumns
} VerticalScrollingModes;

typedef struct
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
} Color;

typedef struct Vdp
{
    struct Genesis* genesis;

    uint8_t* vram;
    uint16_t* vsram;

    // We directly store the CRAM data as decoded 8-bit colors.
    Color* cram;

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
    bool hblank_interrupt_enabled;
    bool hv_counter_latched;

    // Register $01
    bool display_enabled;
    bool vblank_interrupt_enabled;
    bool dma_enabled;
    uint8_t display_height;

    // Register $02
    uint32_t plane_a_nametable;

    // Register $03
    uint32_t window_nametable;

    // Register $04
    uint32_t plane_b_nametable;

    // Register $05
    uint32_t sprites_attribute_table;

    // Register $07
    uint8_t background_color_palette;
    uint8_t background_color_entry;

    // Register $0A
    uint8_t hblank_line;

    // Register $0B
    VerticalScrollingModes vertical_scrolling_mode;
    HorizontalScrollingModes horizontal_scrolling_mode;

    // Register $0C
    uint8_t display_width;
    bool shadow_highlight_enabled;
    int interlace_mode;

    // Register $0D
    uint32_t horizontal_scrolltable;

    // Register $0F
    int auto_increment;

    // Register $10
    uint8_t plane_width;
    uint8_t plane_height;

    // Register $11
    bool window_plane_horizontal_direction; // false: left, true: right
    int window_plane_horizontal_offset;

    // Register $12
    bool window_plane_vertical_direction; // false: up, true: down
    int window_plane_vertical_offset;

    // Register $13 - $14
    uint16_t dma_length;

    // Register $15 - $17
    uint16_t dma_source_address_lo;
    uint8_t dma_source_address_hi;
    int dma_type;

    // In practice, we extract the internal state changes from
    // each write to the VDP's registers because they are more 
    // often used internally than overwritten by the program.
    // Still, keeping the raw values is handy for debugging purposes.
    uint8_t register_raw_values[0x18];

    bool dma_in_progress;
    bool hblank_in_progress;
    bool vblank_in_progress;
    bool vblank_pending; // TODO not implemented, how long does this last?

    // HV counter components
    uint16_t h_counter;
    uint16_t v_counter;

    // Current value of the H-blank counter
    // (will be reloaded with the value stored in register 0xA)
    uint8_t hblank_counter;

    // Video output
    uint8_t* output_buffer;
} Vdp;

Vdp* vdp_make(struct Genesis* cpu);
void vdp_free(Vdp*);

uint16_t vdp_read_data(Vdp* v);
uint8_t vdp_read_data_hi(Vdp*);
uint8_t vdp_read_data_lo(Vdp*);
void vdp_write_data(Vdp*, uint16_t value); // TODO 8bit writes OK but only 16bit reads?
                                           // If confirmed, just use a single read_xxx, and return nibbles in m68k_io

uint16_t vdp_read_control(Vdp*);
void vdp_write_control(Vdp*, uint16_t value);

uint16_t vdp_get_hv_counter(Vdp*); // Get the current value of the HV counter
void vdp_get_resolution(Vdp*, uint16_t* width, uint16_t* height);
void vdp_get_plane_cell_data(Vdp* v, Planes plane, uint16_t cell_index, uint16_t* pattern_index, uint16_t* palette, bool* priority, bool* horizontal_flip, bool* vertical_flip);

void vdp_draw_scanline(Vdp*, int scanline);
void vdp_draw_pattern(Vdp*, uint16_t pattern_index, Color* palette, uint8_t* buffer, uint32_t buffer_width, uint32_t x, uint32_t y, bool horizontal_flip, bool vertical_flip);
void vdp_draw_plane(Vdp*, Planes plane, uint8_t* buffer, uint32_t buffer_width);
