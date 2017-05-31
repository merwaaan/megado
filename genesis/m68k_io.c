#include <m68k/m68k.h>
#include <stdio.h>

#include "genesis.h"
#include "joypad.h"
#include "vdp.h"

#define GENESIS(m) ((Genesis*) (m)->user_data)

bool z80_stopped;

uint8_t m68k_read_b(M68k* m, uint32_t address)
{
    address &= 0xFFFFFF; // 24-bit address bus

    switch (address)
    {

        // https://wiki.megadrive.org/index.php?title=IO_Registers

    case 0xA10000: // Version port
    case 0xA10001:
        return
            (GENESIS(m)->region != Region_Japan) << 7 | // Domestic (0) / Export (1)
            (GENESIS(m)->region == Region_Europe) << 6 | // NTSC (0) / PAL (1)
            false << 5 | // Sega CD connected
            false; // Version

    case 0xA10002:
    case 0xA10003:
        return joypad_read(GENESIS(m)->joypad);
        break;

            // TODO temp, stub z80
    case 0xA11100:
        return !z80_stopped;

    case 0xA11200:
        return 0;

    case 0xC00000: // VDP data port
    case 0xC00002:
        return BYTE_HI(vdp_read_data(GENESIS(m)->vdp)); // TODO add direct read 16b read in m68k_read_w
    case 0xC00001:
    case 0xC00003:
        return BYTE_LO(vdp_read_data(GENESIS(m)->vdp));

    case 0xC00004: // VDP control port
    case 0xC00006:
        return BYTE_HI(vdp_read_control(GENESIS(m)->vdp));
    case 0xC00005:
    case 0xC00007:
        return BYTE_LO(vdp_read_control(GENESIS(m)->vdp));

    case 0xC00008:
        return BYTE_HI(vdp_get_hv_counter(GENESIS(m)->vdp));
    case 0xC00009:
        return BYTE_LO(vdp_get_hv_counter(GENESIS(m)->vdp));

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
    address &= 0xFFFFFF; // 24-bit address bus

    // Cannot write to ROM
    if (address <= 0x3FFFFF)
        return;

    if (address == 0xA10002 || address == 0xA10003)
        joypad_write(GENESIS(m)->joypad, value);

    // Stubbed z80 control
    else if (address == 0xA11100)
    {
        if (value == 0)
            z80_stopped = false;
        else /*(value == 0x100) TODO must be exactly 0x100? */
            z80_stopped = true;
    }

    // " Writing to the VDP control or data ports is interpreted as a 16-bit
    //   write, with the LSB duplicated in the MSB"
    // http://www.tmeeco.eu/BitShit/CMDHW.TXT
    else if (address == 0xC00000)
    {
        vdp_write_data(GENESIS(m)->vdp, value & (value << 8));
    }
    else if (address == 0xC00004)
    {
        vdp_write_control(GENESIS(m)->vdp, value & (value << 8));
    }

    else if (address > 0x82e0 && address < 0x82ff)
        printf("");

    GENESIS(m)->memory[address & 0xFFFFFF] = value;
}

void m68k_write_w(M68k* m, uint32_t address, uint16_t value)
{
    address &= 0xFFFFFF;

    switch (address)
    {
    case 0xC00000: // VDP data port
    case 0xC00002:
        vdp_write_data(GENESIS(m)->vdp, value);
        break;

    case 0xC00004: // VDP control port
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
