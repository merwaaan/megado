#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "z80.h"

struct DecodedInstruction;
struct Joypad;
struct M68k;
struct Renderer;
struct Vdp;

typedef enum {
    Status_NoGameLoaded,
    Status_Pause,
    Status_Running
} Status;

typedef enum {
    Region_Japan,
    Region_Europe,
    Region_USA
} Regions;

typedef struct {
    uint8_t* memory;

    struct M68k* m68k;
    struct Z80* z80;
    struct Vdp* vdp;
    struct Joypad* joypad;

    struct Renderer* renderer;

    Status status;
    Regions region;
} Genesis;

Genesis* genesis_make();
void genesis_free(Genesis*);

void genesis_load_rom_file(Genesis* g, const char* path);
void genesis_initialize(Genesis* g); // TODO rename to reset?

struct DecodedInstruction* genesis_decode(Genesis* g, uint32_t pc);

void genesis_update(Genesis* g);
void genesis_step(Genesis* g);
void genesis_frame(Genesis* g);
