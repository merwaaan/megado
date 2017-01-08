#include <m68k/m68k.h>

#include "genesis.h"

#define GENESIS(m) ((Genesis*) (m)->user_data)

uint8_t m68k_read_b(M68k* m, uint32_t address)
{
    return GENESIS(m)->memory[address];
}

uint16_t m68k_read_w(M68k* m, uint32_t address)
{
    return
        GENESIS(m)->memory[address] << 8 |
        GENESIS(m)->memory[address + 1];
}

uint32_t m68k_read_l(M68k* m, uint32_t address)
{
    return
        GENESIS(m)->memory[address] << 24 |
        GENESIS(m)->memory[address + 1] << 16 |
        GENESIS(m)->memory[address + 2] << 8 |
        GENESIS(m)->memory[address + 3];
}

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
    default:
        return 0xFF; // TODO error?
    }
}

void m68k_write_b(M68k* m, uint32_t address, uint8_t value)
{
    GENESIS(m)->memory[address] = value;
}

void m68k_write_w(M68k* m, uint32_t address, uint16_t value)
{
    GENESIS(m)->memory[address] = (value & 0xFF00) >> 8;
    GENESIS(m)->memory[address + 1] = value & 0xFF;
}

void m68k_write_l(M68k* m, uint32_t address, uint32_t value)
{
    GENESIS(m)->memory[address] = (value & 0xFF000000) >> 24;
    GENESIS(m)->memory[address + 1] = (value & 0xFF0000) >> 16;
    GENESIS(m)->memory[address + 2] = (value & 0xFF00) >> 8;
    GENESIS(m)->memory[address + 3] = value & 0xFF;
}

void m68k_write(M68k* m, Size size, uint32_t address, uint32_t value)
{
    switch (size)
    {
    case Byte:
        m68k_write_b(m, address, value);
    case Word:
        m68k_write_w(m, address, value);
    case Long:
        m68k_write_l(m, address, value);
    }
}
