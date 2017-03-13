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
        return;
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

uint32_t genesis_step(Genesis* g)
{
    uint32_t pc = m68k_step(g->m68k);

    vdp_draw(g->vdp);

    // Handle keyboard input
    SDL_PollEvent(&event);
    switch (event.type)
    {
    case SDL_KEYDOWN:
        switch (event.key.keysym.sym)
        {
        case SDLK_LEFT: joypad_press(g->joypad, Left); break;
        case SDLK_RIGHT: joypad_press(g->joypad, Right); break;
        case SDLK_UP: joypad_press(g->joypad, Up); break;
        case SDLK_RETURN: joypad_press(g->joypad, Start); break;
        case SDLK_a: joypad_press(g->joypad, ButtonA); break;
        case SDLK_z: joypad_press(g->joypad, ButtonB); break;
        case SDLK_e: joypad_press(g->joypad, ButtonC); break;
        }
    }

    return pc;
}

uint8_t* genesis_memory(Genesis* g) { return g->memory; }
M68k* genesis_m68k(Genesis* g) { return g->m68k; }