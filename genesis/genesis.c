#include <m68k/m68k.h>
#include <m68k/instruction.h>
#include <SDL.h>
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
    g->vdp = vdp_make(g->m68k);
    g->joypad = joypad_make();
    g->renderer = renderer_make(g->vdp);

    // Store a pointer to the Genesis instance in the M68k
    // In this way, the various modules can be accessed from M68k-centric I/O functions (see m68k_io.c)
    g->m68k->user_data = g;

    g->region = Region_Europe;

    return g;
}

void genesis_free(Genesis* g)
{
    if (g == NULL)
        return;

    m68k_free(g->m68k);
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

void genesis_load_rom_file(Genesis* g, char* path)
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

    // Display the ROM info stored in the header
    printf("----------------\n");
    print_header_info("", g->memory + 0x100, 16);
    print_header_info("", g->memory + 0x110, 16);
    print_header_info("[Domestic title]", g->memory + 0x120, 48);
    print_header_info("[International title]", g->memory + 0x150, 48);
    print_header_info("[Serial number]", g->memory + 0x180, 8);
    print_header_info("[Country]", g->memory + 0x1F0, 8);
    printf("----------------\n");
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
}

SDL_Event event;

bool genesis_run_frame(Genesis* g)
{
    // Handle keyboard input
    SDL_PollEvent(&event);
    switch (event.type)
    {
    case SDL_KEYDOWN:
        switch (event.key.keysym.scancode)
        {
        case SDL_SCANCODE_LEFT: joypad_press(g->joypad, Left); break;
        case SDL_SCANCODE_RIGHT: joypad_press(g->joypad, Right); break;
        case SDL_SCANCODE_UP: joypad_press(g->joypad, Up); break;
        case SDL_SCANCODE_DOWN: joypad_press(g->joypad, Down); break;
        case SDL_SCANCODE_RETURN: joypad_press(g->joypad, Start); break;
        case SDL_SCANCODE_Q: joypad_press(g->joypad, ButtonA); break;
        case SDL_SCANCODE_W: joypad_press(g->joypad, ButtonB); break;
        case SDL_SCANCODE_E: joypad_press(g->joypad, ButtonC); break;
        }
        break;
    case SDL_KEYUP:
        switch (event.key.keysym.scancode)
        {
        case SDL_SCANCODE_ESCAPE: return false;
        case SDL_SCANCODE_F1: renderer_toggle_palette_window(g->renderer, g->renderer->palette_window == NULL); break;
        case SDL_SCANCODE_F2: renderer_toggle_patterns_window(g->renderer, g->renderer->patterns_window == NULL); break;
        case SDL_SCANCODE_F3: renderer_toggle_planes_window(g->renderer, g->renderer->planes_window == NULL); break;
        case SDL_SCANCODE_TAB: if (g->renderer->planes_window != NULL) renderer_cycle_plane(g->renderer); break;

        case SDL_SCANCODE_LEFT: joypad_release(g->joypad, Left); break;
        case SDL_SCANCODE_RIGHT: joypad_release(g->joypad, Right); break;
        case SDL_SCANCODE_UP: joypad_release(g->joypad, Up); break;
        case SDL_SCANCODE_DOWN: joypad_release(g->joypad, Down); break;
        case SDL_SCANCODE_RETURN: joypad_release(g->joypad, Start); break;
        case SDL_SCANCODE_Q: joypad_release(g->joypad, ButtonA); break;
        case SDL_SCANCODE_W: joypad_release(g->joypad, ButtonB); break;
        case SDL_SCANCODE_E: joypad_release(g->joypad, ButtonC); break;
        }
        break;

    case SDL_QUIT: return false;
    }

    // The number of scanlines depends on the region
    // http://forums.sonicretro.org/index.php?showtopic=5615
    uint16_t lines = g->region == Region_Europe ? 312 : 262;

    for (uint16_t line = 0; line < lines; ++line)
    {
        // Execute one scanline worth of instructions
        m68k_run_cycles(g->m68k, 488); // TODO not sure about that value

        // Draw the scanline
        vdp_draw_scanline(g->vdp, line);
    }

    renderer_render(g->renderer);

    return true;
}

uint8_t* genesis_memory(Genesis* g) { return g->memory; }
M68k* genesis_m68k(Genesis* g) { return g->m68k; }
