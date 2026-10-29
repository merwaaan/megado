#include <m68k/m68k.h>
#include <stdio.h>
#include <stdlib.h>
#include <utils.h>

#include "globals.h"

uint32_t m68k_read(M68k* m, Size size, uint32_t address)
{
    switch (size)
    {
    case Byte:
        return m68k_read_b(m, address);
    case Word:
        return m68k_read_w(m, address);
    case Long:
        return m68k_read_l(m, address);
    case InvalidSize:
    default:
        FATAL("Invalid size %x", size);
    }
}

uint8_t m68k_read_b(M68k* m, uint32_t address)
{
    return memory[address];
}

uint16_t m68k_read_w(M68k* m, uint32_t address)
{
    return m68k_read_b(m, address) << 8 |
           m68k_read_b(m, address + 1);
}

uint32_t m68k_read_l(M68k* m, uint32_t address)
{
    return m68k_read_b(m, address) << 24 |
           m68k_read_b(m, address + 1) << 16 |
           m68k_read_b(m, address + 2) << 8 |
           m68k_read_b(m, address + 3);
}

void m68k_write(M68k* m, Size size, uint32_t address, uint32_t value)
{
    switch (size)
    {
    case Byte:
        m68k_write_b(m, address, value);
        break;
    case Word:
        m68k_write_w(m, address, value);
        break;
    case Long:
        m68k_write_l(m, address, value);
        break;
    case InvalidSize:
    default:
        FATAL("Invalid size %x", size);
    }
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
