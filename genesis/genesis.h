#pragma once

#include <stdint.h>

struct M68k;
struct DecodedInstruction;

typedef struct {
    uint8_t* memory;
    struct M68k* m68k;
} Genesis;

Genesis* genesis_make();
void genesis_free(Genesis*);

void genesis_load_rom_file(Genesis* g, char* path);
void genesis_load_rom_data(Genesis* g, uint8_t* data);

struct DecodedInstruction* genesis_decode(Genesis* g, uint32_t pc);

// Setup the Genesis with the data in the ROM header
void genesis_setup(Genesis* g);

// Step forward one instruction and return the current program counter value
uint32_t genesis_step(Genesis* g);

// We have to use those to get pointers to different fields of the Genesis struct
// TODO really necessary? Can't we just compute pointer offsets on the JS side?
uint8_t* genesis_memory(Genesis*);
struct M68k* genesis_m68k(Genesis*);
