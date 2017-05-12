#include <m68k/m68k.h>
#include <m68k/instruction.h>
#include <stdio.h>
#include <stdlib.h>

#include "genesis.h"
#include "joypad.h"
#include "vdp.h"

Genesis* genesis_make()
{
    Genesis* g = calloc(1, sizeof(Genesis));
    g->memory = calloc(0x1000000, sizeof(uint8_t));
    g->m68k = m68k_make();
    g->vdp = vdp_make(g->m68k);
    g->joypad = joypad_make();

    // Store a pointer to the Genesis instance in the M68k
    // In this way, the various modules can be accessed from M68k-centric I/O functions (see m68k_io.c)
    g->m68k->user_data = g;

    g->region = USA;

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

void genesis_load_rom_file(Genesis* g, char* path)
{
    FILE* file = fopen(path, "rb");
    if (!file)
    {
        printf("Cannot read file \"%s\"", path);
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    int file_length = ftell(file);
    fseek(file, 0, SEEK_SET);

    fread(g->memory, sizeof(uint8_t), file_length, file);

    fclose(file);
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
    uint16_t lines = g->region == Europe ? 312 : 262;

    for (uint16_t line = 0; line < lines; ++line)
    {
        // Execute one scanline worth of instructions
        m68k_run_cycles(g->m68k, 488); // TODO not sure about that value

        // Draw the scanline
        vdp_draw_scanline(g->vdp, line);
    }

    return true;
}

uint8_t* genesis_memory(Genesis* g) { return g->memory; }
M68k* genesis_m68k(Genesis* g) { return g->m68k; }
