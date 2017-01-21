#include <m68k/m68k.h>

#include "genesis.h"
#include "vdp.h"

#define GENESIS(m) ((Genesis*) (m)->user_data)

uint8_t m68k_read_b(M68k* m, uint32_t address)
{
    switch (address)
    {
        // VDP data port
    case 0xC00000:
    case 0xC00002:
        return vdp_read_data_hi(GENESIS(m)->vdp);
    case 0xC00001:
    case 0xC00003:
        return vdp_read_data_lo(GENESIS(m)->vdp);

        // VDP control port
    case 0xC00004:
    case 0xC00006:
        return BYTE_HI(vdp_read_control(GENESIS(m)->vdp));
    case 0xC00005:
    case 0xC00007:
        return BYTE_LO(vdp_read_control(GENESIS(m)->vdp));

    default:
        return GENESIS(m)->memory[address];
    }
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
        m68k_read_w(m, address) << 16 |
        m68k_read_w(m, address + 2);
}

void m68k_write_b(M68k* m, uint32_t address, uint8_t value)
{
    // TODO Byte-wide writes quirks for VDP ???
    // http://www.tmeeco.eu/BitShit/CMDHW.TXT

    GENESIS(m)->memory[address] = value;
}

void m68k_write_w(M68k* m, uint32_t address, uint16_t value)
{
    switch (address)
    {
        // VDP data port
    case 0xC00000:
    case 0xC00002:
        vdp_write_data(GENESIS(m)->vdp, value);
        break;

        // VDP control port
    case 0xC00004:
    case 0xC00006:
        vdp_write_control(GENESIS(m)->vdp, value);
        break;

    default:
        m68k_write_b(m, address, (value & 0xFF00) >> 8);
        m68k_write_b(m, address + 1, value & 0xFF);
    }
}

void m68k_write_l(M68k* m, uint32_t address, uint32_t value)
{
    m68k_write_w(m, address, (value & 0xFFFF0000) >> 16);
    m68k_write_w(m, address + 2, value & 0xFFFF);
}
