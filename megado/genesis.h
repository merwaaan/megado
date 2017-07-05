#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "z80.h"

struct DecodedInstruction;
struct Joypad;
struct M68k;
struct Renderer;
struct Settings;
struct Vdp;

typedef enum {
    Status_NoGameLoaded,
    Status_Pause,
    Status_Running,
    Status_Quitting
} Status;

typedef enum {
    Region_Japan,
    Region_Europe,
    Region_USA
} Regions;

typedef struct Genesis {
    uint8_t* memory;

    struct M68k* m68k;
    struct Z80* z80;
    struct Vdp* vdp;
    struct Joypad* joypad;

    struct Renderer* renderer;
    struct Settings* settings;

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

// Return the name of the game currently being executed as 
// stored in the ROM header. The international name is looked 
// for first and the domestic name is used as a fallback.
//
// Note: the provided buffer must be at least 49 characters 
// long to accomodate all titles plus a terminator character.
void genesis_get_rom_name(Genesis* g, char* name);
