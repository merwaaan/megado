#include <stdio.h>
#include <stdlib.h>

#include "../genesis.h"
#include "../joypad.h"
#include "../psg.h"
#include "../utils.h"
#include "../vdp.h"
#include "../z80.h"
#include "m68k.h"
#include "map.h"

/*
 * Read / Write handlers
 */

// Unmapped

static uint8_t page_unmapped_read_b(Map* m, uint32_t address)
{
    printf("WARNING read from unmapped address: %0X\n", address);

    return 0;
}

static uint16_t page_unmapped_read_w(Map* m, uint32_t address)
{
    printf("WARNING read from unmapped address: %0X\n", address);

    return 0;
}

static void page_unmapped_write_b(Map* m, uint32_t address, uint8_t value)
{
    printf("WARNING write to unmapped address: %0X @ %0X\n", value, address);
}

static void page_unmapped_write_w(Map* m, uint32_t address, uint16_t value)
{
    printf("WARNING write to unmapped address: %0X @ %0X\n", value, address);
}

// Cartridge ROM/RAM (0x000000 - 0x3FFFFF)

static uint8_t page_rom_read_b(Map* m, uint32_t address)
{
    return m->rom[address];
}

static uint16_t page_rom_read_w(Map* m, uint32_t address)
{
    return READ_BE_W(m->rom + address);
}

static void page_rom_write_b(Map* m, uint32_t address, uint8_t value)
{
    printf("WARNING attempted to write to rom: %0X @ %0X\n", value, address);
}

static void page_rom_write_w(Map* m, uint32_t address, uint16_t value)
{
    printf("WARNING attempted to write to rom: %0X @ %0X\n", value, address);
}

// Cartridge SRAM
//
// It lives inside the ROM area but does not necessarily cover whole pages,
// so the pages it touches fallback to the ROM outside of its range.

static bool in_sram(Map* m, uint32_t address)
{
    return m->sram != NULL && address >= m->sram_start && address <= m->sram_end;
}

static uint8_t sram_read_b(Map* m, uint32_t address)
{
    if (in_sram(m, address))
    {
        return m->sram[address - m->sram_start];
    }

    return address <= 0x3FFFFF ? page_rom_read_b(m, address) : page_unmapped_read_b(m, address);
}

static uint16_t sram_read_w(Map* m, uint32_t address)
{
    return sram_read_b(m, address) << 8 | sram_read_b(m, address + 1);
}

static void sram_write_b(Map* m, uint32_t address, uint8_t value)
{
    if (in_sram(m, address))
    {
        m->sram[address - m->sram_start] = value;
    }
    else if (address <= 0x3FFFFF)
    {
        page_rom_write_b(m, address, value);
    }
    else
    {
        page_unmapped_write_b(m, address, value);
    }
}

static void sram_write_w(Map* m, uint32_t address, uint16_t value)
{
    sram_write_b(m, address, (value & 0xFF00) >> 8);
    sram_write_b(m, address + 1, value & 0xFF);
}

// Z80 address space (0xA00000 - 0xA0FFFF)

static uint8_t page_z80_read_b(Map* m, uint32_t address)
{
    return z80_read(m->z80, address & 0xFFFF);
}

static uint16_t page_z80_read_w(Map* m, uint32_t address)
{
    return page_z80_read_b(m, address) << 8 | page_z80_read_b(m, address + 1);
}

static void page_z80_write_b(Map* m, uint32_t address, uint8_t value)
{
    z80_write(m->z80, address & 0xFFFF, value);
}

static void page_z80_write_w(Map* m, uint32_t address, uint16_t value)
{
    page_z80_write_b(m, address, (value & 0xFF00) >> 8);
    page_z80_write_b(m, address + 1, value & 0xFF);
}

// I/O registers and Z80 control (0xA10000 - 0xA1FFFF)
//
// https://wiki.megadrive.org/index.php?title=IO_Registers

static uint8_t page_io_read_b(Map* m, uint32_t address)
{
    switch (address)
    {
    case 0xA10000: // Version port
    case 0xA10001:
        return (m->region != Region_Japan) << 7 |  // Domestic (0) / Export (1)
               (m->region == Region_Europe) << 6 | // NTSC (0) / PAL (1)
               false << 5 |                        // Sega CD connected
               false;                              // Version (bits 3-0)

    case 0xA10002:
    case 0xA10003:
        return joypad_read(m->joypad1);

    case 0xA10004:
    case 0xA10005:
        return joypad_read(m->joypad2);

    // Z80 BUSREQ
    case 0xA11100:
    case 0xA11101:
        // The 68000 has the bus if the Z80 is not running (0: has the bus)
        return z80_bus_ack(m->z80);

    case 0xA11200:
        return 0;

    default:
        return page_unmapped_read_b(m, address);
    }
}

static uint16_t page_io_read_w(Map* m, uint32_t address)
{
    return page_io_read_b(m, address) << 8 | page_io_read_b(m, address + 1);
}

static void page_io_write_b(Map* m, uint32_t address, uint8_t value)
{
    switch (address)
    {
    case 0xA10002:
    case 0xA10003:
        joypad_write(m->joypad1, value);
        break;

    case 0xA10004:
    case 0xA10005:
        joypad_write(m->joypad2, value);
        break;

    case 0xA11100: // Z80 BUSREQ
        z80_bus_req(m->z80, value);
        break;

    case 0xA11200: // Z80 RESET
        z80_reset(m->z80, value);
        break;

    default:
        page_unmapped_write_b(m, address, value);
    }
}

static void page_io_write_w(Map* m, uint32_t address, uint16_t value)
{
    switch (address)
    {
    case 0xA11100:
        z80_bus_req(m->z80, value >> 8);
        break;

    case 0xA11200:
        z80_reset(m->z80, value >> 8);
        break;

    default:
        page_io_write_b(m, address, (value & 0xFF00) >> 8);
        page_io_write_b(m, address + 1, value & 0xFF);
    }
}

// VDP and PSG (0xC00000 - 0xC0FFFF)

static uint8_t page_vdp_read_b(Map* m, uint32_t address)
{
    switch (address)
    {
    case 0xC00000: // VDP data port
    case 0xC00002:
        return BYTE_HI(vdp_read_data(m->vdp));
    case 0xC00001:
    case 0xC00003:
        return BYTE_LO(vdp_read_data(m->vdp));

    case 0xC00004: // VDP control port
    case 0xC00006:
        return BYTE_HI(vdp_read_control(m->vdp));
    case 0xC00005:
    case 0xC00007:
        return BYTE_LO(vdp_read_control(m->vdp));

    case 0xC00008:
        return BYTE_HI(vdp_get_hv_counter(m->vdp));
    case 0xC00009:
        return BYTE_LO(vdp_get_hv_counter(m->vdp));

    default:
        return page_unmapped_read_b(m, address);
    }
}

// Ports are read once per word access, reading them byte by byte would
// trigger their side effects (e.g. advancing the VDP address) twice

static uint16_t page_vdp_read_w(Map* m, uint32_t address)
{
    switch (address)
    {
    case 0xC00000:
    case 0xC00002:
        return vdp_read_data(m->vdp);

    case 0xC00004:
    case 0xC00006:
        return vdp_read_control(m->vdp);

    case 0xC00008:
        return vdp_get_hv_counter(m->vdp);

    default:
        return page_unmapped_read_w(m, address);
    }
}

static void page_vdp_write_b(Map* m, uint32_t address, uint8_t value)
{
    switch (address)
    {
    // " Writing to the VDP control or data ports is interpreted as a 16-bit write, with the LSB duplicated in the MSB"
    // http://www.tmeeco.eu/BitShit/CMDHW.TXT
    case 0xC00000:
        vdp_write_data(m->vdp, value | (value << 8));
        break;

    case 0xC00004:
        vdp_write_control(m->vdp, value | (value << 8));
        break;

    case 0xC00011:
    case 0xC00013:
    case 0xC00015:
    case 0xC00017:
        psg_write(m->psg, value);
        break;

    default:
        page_unmapped_write_b(m, address, value);
    }
}

static void page_vdp_write_w(Map* m, uint32_t address, uint16_t value)
{
    switch (address)
    {
    case 0xC00000: // VDP data port
    case 0xC00002:
        vdp_write_data(m->vdp, value);
        break;

    case 0xC00004: // VDP control port
    case 0xC00006:
        vdp_write_control(m->vdp, value);
        break;

    default:
        page_vdp_write_b(m, address, (value & 0xFF00) >> 8);
        page_vdp_write_b(m, address + 1, value & 0xFF);
    }
}

// RAM (0xE00000 - 0xFFFFFF)
//
// Officially from 0xFF0000 to 0xFFFFFF, in practice mirrored every 64 kb from 0xE00000
// TODO source?

static uint8_t page_ram_read_b(Map* m, uint32_t address)
{
    return m->ram[address & 0xFFFF];
}

static uint16_t page_ram_read_w(Map* m, uint32_t address)
{
    return READ_BE_W(m->ram + (address & 0xFFFF));
}

static void page_ram_write_b(Map* m, uint32_t address, uint8_t value)
{
    m->ram[address & 0xFFFF] = value;
}

static void page_ram_write_w(Map* m, uint32_t address, uint16_t value)
{
    uint8_t* p = m->ram + (address & 0xFFFF);
    p[0] = (value & 0xFF00) >> 8;
    p[1] = value & 0xFF;
}

// Helper to repeat a page across a range
static void map_set(Map* m, uint8_t first_page, uint8_t last_page, Page page)
{
    for (int i = first_page; i <= last_page; ++i)
    {
        m->pages[i] = page;
    }
}

/*
 * Implementation
 */

Map* map_make(uint8_t* rom, uint8_t* ram, struct Z80* z80, struct Vdp* vdp, struct PSG* psg, struct Joypad* joypad1, struct Joypad* joypad2)
{
    Map* map = calloc(1, sizeof(Map));
    map->rom = rom;
    map->ram = ram;
    map->z80 = z80;
    map->vdp = vdp;
    map->psg = psg;
    map->joypad1 = joypad1;
    map->joypad2 = joypad2;

    // Initialize as unmapped
    map_set(map, 0x00, 0xFF, (Page){page_unmapped_read_b, page_unmapped_write_b, page_unmapped_read_w, page_unmapped_write_w});

    return map;
}

void map_init(Map* m, uint8_t* sram, uint32_t sram_start, uint32_t sram_end, Regions region)
{
    m->sram = sram;
    m->sram_start = sram_start;
    m->sram_end = sram_end;
    m->region = region;

    map_set(m, 0x00, 0xFF, (Page){page_unmapped_read_b, page_unmapped_write_b, page_unmapped_read_w, page_unmapped_write_w});
    map_set(m, 0x00, 0x3F, (Page){page_rom_read_b, page_rom_write_b, page_rom_read_w, page_rom_write_w});

    if (sram != NULL)
    {
        map_set(m, (sram_start & M68K_ADDRESS_WIDTH) >> 16, (sram_end & M68K_ADDRESS_WIDTH) >> 16, (Page){sram_read_b, sram_write_b, sram_read_w, sram_write_w});
    }

    map_set(m, 0xA0, 0xA0, (Page){page_z80_read_b, page_z80_write_b, page_z80_read_w, page_z80_write_w});
    map_set(m, 0xA1, 0xA1, (Page){page_io_read_b, page_io_write_b, page_io_read_w, page_io_write_w});
    map_set(m, 0xC0, 0xC0, (Page){page_vdp_read_b, page_vdp_write_b, page_vdp_read_w, page_vdp_write_w});
    map_set(m, 0xE0, 0xFF, (Page){page_ram_read_b, page_ram_write_b, page_ram_read_w, page_ram_write_w});
}

void map_free(Map* map)
{
    if (map == NULL)
    {
        return;
    }

    free(map);
}

uint32_t map_read(Map* m, Size size, uint32_t address)
{
    switch (size)
    {
    case Byte:
        return map_read_b(m, address);
    case Word:
        return map_read_w(m, address);
    case Long:
        return map_read_l(m, address);
    case InvalidSize:
    default:
        FATAL("Invalid size %x", size);
    }
}

void map_write(Map* m, Size size, uint32_t address, uint32_t value)
{
    switch (size)
    {
    case Byte:
        map_write_b(m, address, value);
        break;
    case Word:
        map_write_w(m, address, value);
        break;
    case Long:
        map_write_l(m, address, value);
        break;
    case InvalidSize:
    default:
        FATAL("Invalid size %x", size);
    }
}

uint8_t map_read_b(Map* m, uint32_t address)
{
    address &= M68K_ADDRESS_WIDTH;
    return m->pages[address >> 16].read_byte(m, address);
}

uint16_t map_read_w(Map* m, uint32_t address)
{
    address &= M68K_ADDRESS_WIDTH;
    return m->pages[address >> 16].read_word(m, address);
}

uint32_t map_read_l(Map* m, uint32_t address)
{
    uint16_t hi = map_read_w(m, address);
    uint16_t lo = map_read_w(m, address + 2);

    return ((uint32_t)hi << 16) | lo;
}

void map_write_b(Map* m, uint32_t address, uint8_t value)
{
    address &= M68K_ADDRESS_WIDTH;
    m->pages[address >> 16].write_byte(m, address, value);
}

void map_write_w(Map* m, uint32_t address, uint16_t value)
{
    address &= M68K_ADDRESS_WIDTH;
    m->pages[address >> 16].write_word(m, address, value);
}

void map_write_l(Map* m, uint32_t address, uint32_t value)
{
    uint16_t hi = (value & 0xFFFF0000) >> 16;
    uint16_t lo = value & 0xFFFF;

    map_write_w(m, address, hi);
    map_write_w(m, address + 2, lo);
}
