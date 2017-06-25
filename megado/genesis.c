#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "genesis.h"
#include "joypad.h"
#include "m68k/m68k.h"
#include "m68k/instruction.h"
#include "renderer.h"
#include "settings.h"
#include "vdp.h"

Genesis* genesis_make()
{
    Genesis* g = calloc(1, sizeof(Genesis));
    g->memory = calloc(0x1000000, sizeof(uint8_t));
    g->m68k = m68k_make(g);
    g->z80 = z80_make();
    g->vdp = vdp_make(g);
    g->joypad = joypad_make();
    g->renderer = renderer_make(g);
    g->settings = settings_make();
    g->status = Status_NoGameLoaded;

    return g;
}

void genesis_free(Genesis* g)
{
    if (g == NULL)
        return;

    // Save the settings when quitting
    // TODO not the best place to that, should rather be in quit() or deinit() or something
    settings_save(g->settings);

    m68k_free(g->m68k);
    z80_free(g->z80);
    vdp_free(g->vdp);
    joypad_free(g->joypad);
    settings_free(g->settings);

    free(g->memory);
    free(g);
}

static void print_header_info(char* label, uint8_t* data, uint8_t length)
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
    case 'U':
    case '4': // TODO sometimes 4 can be found as the region code in (U) roms, not sure about the validity of that
        g->region = Region_USA; break;

    case 'A': // Maui Mallard has this country code and apparently it's for 'PAL
              // and french SECAM'
        g->region = Region_Europe; break;

    default:
        printf("Invalid country code, using default (Japan)");
        g->region = Region_Japan;
    }
    // TODO it seems possible to have multiple country codes, how to handle that?
    // TODO found something interesting, try that: http://mode5.net/32x-DDK/Bulletins/Gen-tech/Tech31-01.gif

    g->status = Status_Running;
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
    // TODO should we reset the VDP?
}

void genesis_update(Genesis* g)
{
    if (g->status == Status_Running)
        genesis_frame(g);

    renderer_render(g->renderer);
}

void genesis_step(Genesis* g)
{
    m68k_step(g->m68k);
}

void genesis_frame(Genesis* g)
{
    // The number of scanlines depends on the region
    // http://forums.sonicretro.org/index.php?showtopic=5615
    uint16_t lines = g->region == Region_Europe ? 312 : 262; // TODO not sure about these values

    for (uint16_t line = 0; line < lines; ++line)
    {
        // Execute one scanline worth of instructions
        m68k_run_cycles(g->m68k, 488); // TODO not sure about that value
        z80_run_cycles(g->z80, 244); // Z80 runs at half the frequency of M68

        // Draw the scanline
        vdp_draw_scanline(g->vdp, line);

        // Exit early if the emulation has been paused
        if (g->status != Status_Running)
            break;

        g->vdp->hblank_in_progress = true;
        m68k_run_cycles(g->m68k, 84); // http://gendev.spritesmind.net/forum/viewtopic.php?t=94#p1105
        z80_run_cycles(g->z80, 42); // Z80 runs at half the frequency of M68
        g->vdp->hblank_in_progress = false;

        // Exit early if the emulation has been paused
        if (g->status != Status_Running)
            break;
    }
}
