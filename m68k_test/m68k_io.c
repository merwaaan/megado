#include <m68k/m68k.h>

#include "globals.h"

#define GENESIS(m) ((Genesis*) (m)->user_data)

uint8_t m68k_read_b(M68k* m, uint32_t address)
{
    return memory[address];
}

uint16_t m68k_read_w(M68k* m, uint32_t address)
{
    return
        m68k_read_b(m, address) << 8 |
        m68k_read_b(m, address + 1);
}

uint32_t m68k_read_l(M68k* m, uint32_t address)
{
    return
        m68k_read_b(m, address) << 24 |
        m68k_read_b(m, address + 1) << 16 |
        m68k_read_b(m, address + 2) << 8 |
        m68k_read_b(m, address + 3);
}

void m68k_write_b(M68k* m, uint32_t address, uint8_t value)
{
    memory[address] = value;
}

void m68k_write_w(M68k* m, uint32_t address, uint16_t value)
{

    m68k_write_b(m, address, (value & 0xFF00) >> 8);
    m68k_write_b(m, address + 1, value & 0xFF);
}

void m68k_write_l(M68k* m, uint32_t address, uint32_t value)
{
    m68k_write_b(m, address, (value & 0xFF000000) >> 24);
    m68k_write_b(m, address + 1, (value & 0xFF0000) >> 16);
    m68k_write_b(m, address + 2, (value & 0xFF00) >> 8);
    m68k_write_b(m, address + 3, value & 0xFF);
}
