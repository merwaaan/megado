#include <m68k/m68k.h>
#include <m68k/map.h>

#include "globals.h"

// Tests run against a flat 16 MB memory, every page points to it

static uint8_t test_read_b(Map* map, uint32_t address)
{
    return memory[address & M68K_ADDRESS_WIDTH];
}

static uint16_t test_read_w(Map* map, uint32_t address)
{
    return test_read_b(map, address) << 8 |
           test_read_b(map, address + 1);
}

static void test_write_b(Map* map, uint32_t address, uint8_t value)
{
    memory[address & M68K_ADDRESS_WIDTH] = value;
}

static void test_write_w(Map* map, uint32_t address, uint16_t value)
{
    test_write_b(map, address, (value & 0xFF00) >> 8);
    test_write_b(map, address + 1, value & 0xFF);
}

// Fake map that just maps to a flat memory buffer
void test_map_init(Map* map)
{
    for (int i = 0; i < 0x100; ++i)
    {
        map->pages[i] = (Page){test_read_b, test_write_b, test_read_w, test_write_w};
    }
}
