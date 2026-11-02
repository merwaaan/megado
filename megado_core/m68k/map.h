#pragma once

#include <stdint.h>

#include "../genesis.h"
#include "bit_utils.h"

struct Joypad;
struct Map;
struct PSG;
struct Vdp;
struct Z80;

// We split the memory map into 256 pages of 0x10000 bytes each, covering all the 24-bit address space.
// That's better for speed than repeatedly evaluating which component should handle each access.
// It's also easier to follow the logic when each region has separate read/write handlers.
typedef struct Page
{
    uint8_t (*read_byte)(struct Map*, uint32_t address);
    void (*write_byte)(struct Map*, uint32_t address, uint8_t value);

    uint16_t (*read_word)(struct Map*, uint32_t address);
    void (*write_word)(struct Map*, uint32_t address, uint16_t value);

    // No long read/write because they can cross page boundaries.
    // We implement them by reading/writing two words instead.
    // TODO is this an issue wrt to r/w side effects? (vdp stuff)
} Page;

typedef struct Map
{
    // Memory
    uint8_t* rom;
    uint8_t* ram;
    uint8_t* sram;
    uint32_t sram_start, sram_end;

    // Memory-mapped devices
    struct Z80* z80;
    struct Vdp* vdp;
    struct PSG* psg;
    struct Joypad* joypad1;
    struct Joypad* joypad2;

    // Exposed by the version register
    Regions region;

    // 256 pages of 0x10000 bytes each, covering all the 24-bit address space
    Page pages[0x100];
} Map;

Map* map_make(uint8_t* rom, uint8_t* ram, struct Z80*, struct Vdp*, struct PSG*, struct Joypad* joypad1, struct Joypad* joypad2);
void map_free(Map*);

// The map must be initialized for each ROM, as the SRAM existence/placement varies
void map_init(Map*, uint8_t* sram, uint32_t sram_start, uint32_t sram_end, Regions region);

// TODO avoid this? bind directly in instructions?
uint32_t map_read(Map*, Size size, uint32_t address);
void map_write(Map*, Size size, uint32_t address, uint32_t value);

uint8_t map_read_b(Map*, uint32_t address);
uint16_t map_read_w(Map*, uint32_t address);
uint32_t map_read_l(Map*, uint32_t address);

void map_write_b(Map*, uint32_t address, uint8_t value);
void map_write_w(Map*, uint32_t address, uint16_t value);
void map_write_l(Map*, uint32_t address, uint32_t value);
