#include <GLFW/glfw3.h>
#include <m68k/m68k.h>
#include <m68k/instruction.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "genesis.h"
#include "joypad.h"
#include "renderer.h"
#include "vdp.h"

Genesis* genesis_make()
{
    Genesis* g = calloc(1, sizeof(Genesis));
    g->memory = calloc(0x1000000, sizeof(uint8_t));
    g->m68k = m68k_make();
    g->z80 = z80_make();
    g->vdp = vdp_make(g->m68k);
    g->joypad = joypad_make();
    g->renderer = renderer_make(g);

    // Store a pointer to the Genesis instance in the M68k.
    // In this way, the various modules can be accessed from M68k-centric I/O functions (see m68k_io.c).
    g->m68k->user_data = g;

    return g;
}

void genesis_free(Genesis* g)
{
    if (g == NULL)
        return;

    m68k_free(g->m68k);
    z80_free(g->z80);
    vdp_free(g->vdp);
    joypad_free(g->joypad);

    free(g->memory);
    free(g);
}

void print_header_info(char* label, uint8_t* data, uint8_t length)
{
    char* string = calloc(length + 1, sizeof(char));
    strncpy(string, data, length);

    printf("%-48s %s\n", string, label);

    free(string);
}

void genesis_load_rom_file(Genesis* g, const char* path)
{
    printf("Opening %s...\n", path);

    FILE* file = fopen(path, "rb");
    if (!file)
    {
        printf("Cannot read file \"%s\"", path);
        exit(1);
    }

    // Load the ROM into memory
    fseek(file, 0, SEEK_END);
    int file_length = ftell(file);
    fseek(file, 0, SEEK_SET);
    fread(g->memory, sizeof(uint8_t), file_length, file);

    fclose(file);

    // Display info from the ROM header
    printf("----------------\n");
    print_header_info("", g->memory + 0x100, 16);
    print_header_info("", g->memory + 0x110, 16);
    print_header_info("[Domestic title]", g->memory + 0x120, 48);
    print_header_info("[International title]", g->memory + 0x150, 48);
    print_header_info("[Serial number]", g->memory + 0x180, 8);
    print_header_info("[Country]", g->memory + 0x1F0, 8);
    printf("----------------\n");

    // Set the system region depending on country code of the game
    uint8_t* country_codes = g->memory + 0x1F0;
    switch (country_codes[0])
    {
    case 'E': g->region = Region_Europe; break;
    case 'J': g->region = Region_Japan; break;
    case 'U': g->region = Region_USA; break;
    default:
        printf("Invalid country code, using default (Japan)");
        g->region = Region_Japan;
    }
    // TODO it seems possible to have multiple country codes, how to handle that?
}

void genesis_load_rom_data(Genesis* g, uint8_t* data)
{
    // TODO
}

struct DecodedInstruction* genesis_decode(Genesis* g, uint32_t pc)
{
    printf(".%p %p.\n", g, g->m68k);
    return m68k_decode(g->m68k, pc);
}

#define WORD(x) ((x)[0] << 8 | (x)[1])
#define LONG(x) ((x)[0] << 24 | (x)[1] << 16 | (x)[2] << 8 | (x)[3])

void genesis_initialize(Genesis* g)
{
    // http://darkdust.net/writings/megadrive/initializing
    // http://md.squee.co/Howto:Initialise_a_Mega_Drive

    m68k_initialize(g->m68k);
    z80_initialize(g->z80);
}

bool genesis_run_frame(Genesis* g)
{
    // The number of scanlines depends on the region
    // http://forums.sonicretro.org/index.php?showtopic=5615
    uint16_t lines = g->region == Region_Europe ? 312 : 262;

    for (uint16_t line = 0; line < lines; ++line)
    {
        // Execute one scanline worth of instructions
        m68k_run_cycles(g->m68k, 488); // TODO not sure about that value
        z80_run_cycles(g->z80, 488);

        // Draw the scanline
        vdp_draw_scanline(g->vdp, line);
    }

    renderer_render(g->renderer);

    return true;
}

uint8_t* genesis_memory(Genesis* g) { return g->memory; }
M68k* genesis_m68k(Genesis* g) { return g->m68k; }
