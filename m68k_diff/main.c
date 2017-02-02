#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "musashi/m68k.h"
#include "m68k/m68k.h" // Names might clash...
// TODO add https://github.com/notaz/cyclone68000 to the list?

// Musashi I/O

uint8_t* memory_musashi;

unsigned int m68k_read_memory_8(unsigned int address)
{
    // Simulate the Z80 as stopped
    if (address == 0xA11100)
        return 0;

    return memory_musashi[address];
}

unsigned int m68k_read_memory_16(unsigned int address)
{
    return m68k_read_memory_8(address) << 8 | m68k_read_memory_8(address + 1);
}

unsigned int m68k_read_memory_32(unsigned int address)
{
    return m68k_read_memory_16(address) << 16 | m68k_read_memory_16(address + 2);
}

void m68k_write_memory_8(unsigned int address, unsigned int value)
{
    memory_musashi[address] = value & 0xFF;
}

void m68k_write_memory_16(unsigned int address, unsigned int value)
{
    memory_musashi[address] = (value & 0xFF00) >> 8;
    memory_musashi[address + 1] = value & 0x00FF;
}

void m68k_write_memory_32(unsigned int address, unsigned int value)
{
    memory_musashi[address] = (value & 0xFF000000) >> 24;
    memory_musashi[address + 1] = (value & 0x00FF0000) << 16;
    memory_musashi[address + 2] = (value & 0x0000FF00) >> 8;
    memory_musashi[address + 3] = value & 0x000000FF;
}

unsigned int m68k_read_disassembler_16(unsigned int address)
{
    return m68k_read_memory_16(address);
}

unsigned int m68k_read_disassembler_32(unsigned int address)
{
    return m68k_read_memory_32(address);
}

void musashi_instr_callback()
{
    static char buff[100];
    static unsigned int pc;
    static unsigned int instr_size;
    pc = m68k_get_reg(NULL, M68K_REG_PC);
    instr_size = m68k_disassemble(buff, pc, M68K_CPU_TYPE_68000);
    printf("E %03x: %s\n", pc, buff);
}

// xxx emulator I/O

uint8_t* memory_xxx;

uint8_t m68k_read_b(M68k* m, uint32_t address)
{
    // Simulate the Z80 as stopped
    if (address == 0xA11100)
        return 0;

    return memory_xxx[address];
}

uint16_t m68k_read_w(M68k* m, uint32_t address)
{
    return m68k_read_b(m, address) << 8 | m68k_read_b(m, address + 1);
}

uint32_t m68k_read_l(M68k* m, uint32_t address)
{
    return m68k_read_w(m, address) << 16 | m68k_read_w(m, address + 2);
}

void m68k_write_b(M68k* m, uint32_t address, uint8_t value)
{
    memory_xxx[address] = value & 0xFF;
}

void m68k_write_w(M68k* m, uint32_t address, uint16_t value)
{
    memory_xxx[address] = (value & 0xFF00) >> 8;
    memory_xxx[address + 1] = value & 0x00FF;
}

void m68k_write_l(M68k* m, uint32_t address, uint32_t value)
{
    memory_xxx[address] = (value & 0xFF000000) >> 24;
    memory_xxx[address + 1] = (value & 0x00FF0000) << 16;
    memory_xxx[address + 2] = (value & 0x0000FF00) >> 8;
    memory_xxx[address + 3] = value & 0x000000FF;
}

// Data dump functions

char* dump_format = "PC %04x, SR %04x, A0 %08x, A1 %08x, A2 %08x, A3 %08x, A4 %08x, A5 %08x, A6 %08x, A7 %08x, D0 %08x, D1 %08x, D2 %08x, D3 %08x, D4 %08x, D5 %08x, D6 %08x, D7 %08x\n";

#define MUSASHI_REG(reg) m68k_get_reg(NULL, M68K_REG_ ## reg)

void musashi_dump(FILE* file)
{
    fprintf(file, dump_format,
        MUSASHI_REG(PC), MUSASHI_REG(SR),
        MUSASHI_REG(A0), MUSASHI_REG(A1), MUSASHI_REG(A2), MUSASHI_REG(A3), MUSASHI_REG(A4), MUSASHI_REG(A5), MUSASHI_REG(A6), MUSASHI_REG(A7),
        MUSASHI_REG(D0), MUSASHI_REG(D1), MUSASHI_REG(D2), MUSASHI_REG(D3), MUSASHI_REG(D4), MUSASHI_REG(D5), MUSASHI_REG(D6), MUSASHI_REG(D7));
}

#define XXX_ADDR_REG(reg) xxx->address_registers[reg]
#define XXX_DATA_REG(reg) xxx->data_registers[reg]

M68k* xxx = NULL;

void xxx_dump(FILE* file)
{
    fprintf(file, dump_format,
        xxx->pc, xxx->status,
        XXX_ADDR_REG(0), XXX_ADDR_REG(1), XXX_ADDR_REG(2), XXX_ADDR_REG(3), XXX_ADDR_REG(4), XXX_ADDR_REG(5), XXX_ADDR_REG(6), XXX_ADDR_REG(7),
        XXX_DATA_REG(0), XXX_DATA_REG(1), XXX_DATA_REG(2), XXX_DATA_REG(3), XXX_DATA_REG(4), XXX_DATA_REG(5), XXX_DATA_REG(6), XXX_DATA_REG(7));
}

#define MAX_ROM_SIZE 0x10000
#define MAX_MEM_SIZE 0x1000000

int main(int argc, char* argv[])
{
    // Arg 1: rom path
    // Arg 2: program start offset (hex)

    memory_musashi = calloc(MAX_MEM_SIZE, sizeof(uint8_t));
    memory_xxx = calloc(MAX_MEM_SIZE, sizeof(uint8_t));

    // Load the rom file

    uint8_t rom[MAX_ROM_SIZE];

    FILE* rom_file = fopen(argv[1], "rb");
    if (rom_file == NULL)
    {
        printf("Cannot open file \"%d\"", argv[1]);
        return 1;
    }

    fread(memory_musashi, MAX_ROM_SIZE, sizeof(uint8_t), rom_file);
    rewind(rom_file);
    fread(memory_xxx, MAX_ROM_SIZE, sizeof(uint8_t), rom_file);
    fclose(rom_file);

    // Setup Musashi
    m68k_init();
    m68k_set_cpu_type(M68K_CPU_TYPE_68000);
    m68k_set_instr_hook_callback(musashi_instr_callback);
    m68k_pulse_reset();

    // Setup xxx
    xxx = m68k_make();

    // Start the program at an optional offset
    if (argc > 2)
    {
        int offset = strtol(argv[2], NULL, 16);

        m68k_set_reg(M68K_REG_PC, offset);
        xxx->pc = offset;
    }

    FILE* dumpfile_musashi = fopen("musashi_dump.txt", "w");
    FILE* dumpfile_xxx = fopen("xxx_dump.txt", "w");

    musashi_dump(dumpfile_musashi);
    xxx_dump(dumpfile_xxx);

    for (int i = 0; i < 100000; ++i)
    {
        // Manual breakpoint!
        if (xxx->pc == 0x250)
            printf("breakpoint\n");

        // Musashi
        m68k_execute(0);
        musashi_dump(dumpfile_musashi);

        // xxx
        m68k_step(xxx);
        xxx_dump(dumpfile_xxx);

        printf("%04x %04x\n", memory_musashi[0xA11100], memory_xxx[0xA11100]);
    }

    free(memory_musashi);
    free(memory_xxx);

    m68k_free(xxx);

    fclose(dumpfile_musashi);
    fclose(dumpfile_xxx);

    return 0;
}
