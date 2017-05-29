#pragma once

#include <stdbool.h>
#include <stdint.h>

struct DecodedInstruction;
struct Joypad;
struct M68k;
struct Renderer;
struct Vdp;

typedef enum {
    Region_Japan,
    Region_Europe,
    Region_USA
} Regions;

typedef struct {
    uint8_t* memory;
    
    struct M68k* m68k;
    struct Vdp* vdp;
    struct Joypad* joypad;

    struct Renderer* renderer;

    Regions region;
} Genesis;

Genesis* genesis_make();
void genesis_free(Genesis*);

void genesis_load_rom_file(Genesis* g, char* path);
void genesis_load_rom_data(Genesis* g, uint8_t* data);

struct DecodedInstruction* genesis_decode(Genesis* g, uint32_t pc);

// Prepare the Genesis for execution
void genesis_initialize(Genesis* g);

bool genesis_run_frame(Genesis* g);

// We have to use those to get pointers to different fields of the Genesis struct
// TODO really necessary? Can't we just compute pointer offsets on the JS side?
uint8_t* genesis_memory(Genesis*);
struct M68k* genesis_m68k(Genesis*);
