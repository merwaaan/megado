// miniz defines zlib-compatible names (Byte, ...) that clash with ours
#define MINIZ_NO_ZLIB_COMPATIBLE_NAMES
#include <miniz.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debugger.h"
#include "genesis.h"
#include "joypad.h"
#include "m68k/m68k.h"
#include "psg.h"
#include "vdp.h"
#include "ym2612.h"
#include "z80.h"

static const uint32_t NTSC_MASTER_FREQUENCY = 53693175;
static const uint32_t PAL_MASTER_FREQUENCY = 53203424;

Genesis* genesis_make()
{
    Genesis* g = calloc(1, sizeof(Genesis));
    g->rom = calloc(0x400000, sizeof(uint8_t));
    g->ram = calloc(0x10000, sizeof(uint8_t));
    g->m68k = m68k_make(g);
    g->z80 = z80_make(g);
    g->vdp = vdp_make(g);
    g->psg = psg_make(g);
    g->ym2612 = ym2612_make(g);
    g->joypad1 = joypad_make();
    g->joypad2 = joypad_make();
    g->debugger = debugger_make(g);
    g->status = Status_NoGameLoaded;

    return g;
}

void genesis_free(Genesis* g)
{
    if (g == NULL)
    {
        return;
    }

    m68k_free(g->m68k);
    z80_free(g->z80);
    vdp_free(g->vdp);
    psg_free(g->psg);
    ym2612_free(g->ym2612);
    joypad_free(g->joypad1);
    joypad_free(g->joypad2);
    debugger_free(g->debugger);
    free(g->rom);
    free(g->ram);
    free(g->sram);

    free(g);
}

static void print_header_info(char* label, uint8_t* data, uint8_t length)
{
    char* string = calloc(length + 1, sizeof(char));
    strncpy(string, (char*)data, length);

    printf("%-48s %s\n", string, label);

    free(string);
}

static void load_rom_data(Genesis* g, const uint8_t* data, size_t size)
{
    if (size > 0x400000)
    {
        size = 0x400000;
    }

    memset(g->rom, 0, 0x400000);
    memcpy(g->rom, data, size);

    genesis_initialize(g);

    // Display info from the ROM header

    printf("----------------\n");
    print_header_info("", g->rom + 0x100, 16);
    print_header_info("", g->rom + 0x110, 16);
    print_header_info("[Domestic title]", g->rom + 0x120, 48);
    print_header_info("[International title]", g->rom + 0x150, 48);
    print_header_info("[Serial number]", g->rom + 0x180, 14);
    print_header_info("[Country]", g->rom + 0x1F0, 8);
    printf("%06x - %06x                                  [ROM]\n", 0, g->rom_end);
    printf("%06x - %06x                                  [SRAM]\n", g->sram_start, g->sram_end);
    printf("----------------\n");

    // Set the system region depending on the country code of the game

    uint8_t* country_codes = g->rom + 0x1F0;
    switch (country_codes[0])
    {

        // Initially, the country code was either 'J', 'U' or 'E'
        // (some games have multiple country codes, we only consider the first one)

    case 'J':
        g->region = Region_Japan;
        break;
    case 'U':
        g->region = Region_USA;
        break;
    case 'E':
        g->region = Region_Europe;
        break;

    default:
    {
        // Specifications were updated for later Genesis versions and a single ASCII character encodes compatible regions
        // (Genesis Technical Bulletin #31, http://mode5.net/32x-DDK/Bulletins/Gen-tech/Tech31-01.gif)

        uint8_t code = strtol((char*)country_codes, NULL, 16);

        if (code & 1)
        {
            g->region = Region_Japan; // Bit 0 for Japan
        }
        else if (code & 4)
        {
            g->region = Region_USA; // BIT 2 for USA
        }
        else if (code & 0xA)
        {
            g->region = Region_Europe; // Bit 1 and 4 for Europe (Bit 1 is documented as "Japan PAL", but in practice it seems to be used for "French PAL Secam")
        }
        else
        {
            printf("Invalid country code %s, using default (Japan)\n", country_codes);
            g->region = Region_Japan;
        }
    }
    }

    g->status = Status_Running;
}

static bool is_zip(const uint8_t* data, size_t size)
{
    return size >= 4 && memcmp(data, "PK\x03\x04", 4) == 0;
}

bool genesis_load_rom(Genesis* g, const uint8_t* data, size_t size)
{
    // Not a zip: load the ROM directly

    if (!is_zip(data, size))
    {
        load_rom_data(g, data, size);
        return true;
    }

    // Zip: extract and load the ROM
    // (archives often bundle extra files next to the ROM so we pick the largest file)

    mz_zip_archive zip = {0};

    if (!mz_zip_reader_init_mem(&zip, data, size, 0))
    {
        printf("Cannot open zip archive\n");
        return false;
    }


    int rom_index = -1;
    mz_uint64 rom_size = 0;

    for (mz_uint i = 0; i < mz_zip_reader_get_num_files(&zip); i++)
    {
        mz_zip_archive_file_stat stat;

        if (mz_zip_reader_file_stat(&zip, i, &stat) && !stat.m_is_directory && stat.m_uncomp_size > rom_size)
        {
            rom_index = (int)i;
            rom_size = stat.m_uncomp_size;
        }
    }

    if (rom_index < 0)
    {
        printf("No ROM found in zip archive\n");
        mz_zip_reader_end(&zip);
        return false;
    }

    size_t rom_data_size;
    void* rom_data = mz_zip_reader_extract_to_heap(&zip, rom_index, &rom_data_size, 0);

    mz_zip_reader_end(&zip);

    if (rom_data == NULL)
    {
        printf("Cannot extract ROM from zip archive\n");
        return false;
    }

    load_rom_data(g, rom_data, rom_data_size);

    mz_free(rom_data);

    return true;
}

struct DecodedInstruction* genesis_decode(Genesis* g, uint32_t pc)
{
    printf(".%p %p.\n", g, g->m68k);
    return m68k_decode(g->m68k, pc);
}

#define WORD(x) ((x)[0] << 8 | (x)[1])
#define LONG(x) ((x)[0] << 24 | (x)[1] << 16 | (x)[2] << 8 | (x)[3])

void genesis_initialize(Genesis* g)
{
    g->remaining_cycles = 0;

    // http://darkdust.net/writings/megadrive/initializing
    // http://md.squee.co/Howto:Initialise_a_Mega_Drive

    g->rom_end = LONG(g->rom + 0x1a4);

    free(g->sram);
    g->sram = NULL;
    g->sram_start = 0;
    g->sram_end = 0;

    if (g->rom[0x1b0] == 'R' && g->rom[0x1b1] == 'A')
    {
        g->sram_start = LONG(g->rom + 0x1b4);
        g->sram_end = LONG(g->rom + 0x1b8);

        if (g->sram_end >= g->sram_start && g->sram_end - g->sram_start < 0x10000)
        {
            g->sram = calloc(g->sram_end - g->sram_start + 1, sizeof(uint8_t));
        }
    }

    m68k_initialize(g->m68k);
    z80_initialize(g->z80);
    vdp_initialize(g->vdp);
    psg_initialize(g->psg);
    ym2612_initialize(g->ym2612);
    debugger_initialize(g->debugger);
}

uint32_t genesis_master_frequency(Genesis* g)
{
    return g->region == Region_Europe ? PAL_MASTER_FREQUENCY : NTSC_MASTER_FREQUENCY;
}

void genesis_step(Genesis* g)
{
    // Run for one instruction

    int16_t cycles = m68k_step(g->m68k);

    // If we hit a breakpoint or there was an error, just stop here

    if (cycles < 0)
    {
        return;
    }

    // We don't need to touch m68k->remaining_master_cycles, because if we had
    // wanted to run for exactly one instruction, we would have called
    // m68k_run_cycles with `cycles * 7`, and the function would have consumed
    // exactly that amount of cycles, so we remaining_master_cycles would not
    // have been affected.

    // Let other systems catch up

    uint32_t master_cycles = cycles * 7;

    z80_run_cycles(g->z80, master_cycles);
    psg_run_cycles(g->psg, master_cycles);
    ym2612_run_cycles(g->ym2612, master_cycles);
}

// Run for a given number of master cycles
static void genesis_run_cycles(Genesis* g, double cycles)
{
    // Master cycles can be fractional here, but subsystems only deal with the integral part, hence this loop

    g->remaining_cycles += cycles;

    while (g->remaining_cycles > 0)
    {
        // The m68k can halt prematurely due to breakpoint

        uint32_t actual_cycles = m68k_run_cycles(g->m68k, cycles);

        // Let the other systems catch up

        z80_run_cycles(g->z80, actual_cycles);
        psg_run_cycles(g->psg, actual_cycles);
        ym2612_run_cycles(g->ym2612, actual_cycles);
        vdp_run_cycles(g->vdp, actual_cycles);

        g->remaining_cycles -= actual_cycles;

        // If we hit a breakpoint, abort

        if (g->status != Status_Running)
        {
            break;
        }
    }
}

uint32_t genesis_run_samples(Genesis* g, int16_t* samples, uint32_t count, double sample_duration)
{
    double d_cycles = sample_duration * genesis_master_frequency(g);

    uint32_t produced = 0;
    while (produced < count && g->status == Status_Running)
    {
        genesis_run_cycles(g, d_cycles);

        // @Temporary: use left channel for PSG and right channel for YM2612
        samples[produced * 2] = psg_mix(g->psg);
        samples[produced * 2 + 1] = ym2612_mix(g->ym2612);
        ++produced;
    }

    return produced;
}

uint8_t* genesis_framebuffer(Genesis* g, uint16_t* width, uint16_t* height)
{
    vdp_get_resolution(g->vdp, width, height);

    return g->vdp->output_buffer;
}

void genesis_set_button(Genesis* g, uint8_t port, uint16_t button, bool pressed)
{
    Joypad* joypad = port == 0 ? g->joypad1 : g->joypad2;

    if (pressed)
    {
        joypad_press(joypad, button);
    }
    else
    {
        joypad_release(joypad, button);
    }
}

void genesis_get_rom_name(Genesis* g, char* name)
{
    // Get the international title from the header

    strncpy(name, (char*)(g->rom + 0x150), 48);

    // Use the domestic title as a fallback

    if (name[0] == ' ')
    {
        strncpy(name, (char*)(g->rom + 0x120), 48);
    }

    // Remove trailing whitespaces

    uint8_t cursor = 48;
    while (name[--cursor] == ' ')
        ;

    name[cursor + 1] = '\0';
}
