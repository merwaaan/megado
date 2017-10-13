#pragma once

#include <stdbool.h>
#include <stdint.h>

struct Debugger;
struct DecodedInstruction;
struct Joypad;
struct M68k;
struct Z80;
struct Renderer;
struct Audio;
struct Settings;
struct Vdp;
struct PSG;
struct YM2612;

typedef enum {
    Status_NoGameLoaded,
    Status_Pause,
    Status_Running,
    Status_Rewinding,
    Status_Quitting
} Status;

typedef enum {
    Region_Japan,
    Region_Europe,
    Region_USA
} Regions;

typedef struct Genesis
{
    double remaining_cycles;

    uint8_t* rom; // Typically 0x000000 - 0x3FFFFF
    uint8_t* ram; // Typically 0xFF0000 - 0xFFFFFF
    uint8_t* sram;

    struct M68k* m68k;
    struct Z80* z80;
    struct Vdp* vdp;
    struct Joypad* joypad1;
    struct Joypad* joypad2;
    struct PSG* psg;
    struct YM2612* ym2612;

    struct Renderer* renderer;
    struct Audio*    audio;
    struct Settings* settings;
    struct Debugger* debugger;

    Status status;
    Regions region;

    // Memory layout
    uint32_t rom_end, sram_start, sram_end;
} Genesis;

Genesis* genesis_make();
void genesis_free(Genesis*);

void genesis_load_rom_file(Genesis* g, const char* path);
void genesis_initialize(Genesis* g);

struct DecodedInstruction* genesis_decode(Genesis* g, uint32_t pc);

void genesis_update(Genesis* g);
void genesis_step(Genesis* g);

uint32_t genesis_master_frequency(Genesis*);

// Return the name of the game currently being executed as
// stored in the ROM header. The international name is looked
// for first and the domestic name is used as a fallback.
//
// Note: the provided buffer must be at least 49 characters
// long to accomodate all titles plus a terminator character.
void genesis_get_rom_name(Genesis* g, char* name);
