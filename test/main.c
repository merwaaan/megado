#include <genesis/genesis.h>
#include <m68k/instruction.h>
#include <m68k/m68k.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <WinSock2.h>
#elif __linux__
#include <arpa/inet.h>
#endif


void check_alignment(Genesis* g)
{
    printf("m68k: %p (%zu)\n", g->m68k, sizeof(g->m68k));
    printf("m68k data regs: %p (%zu)\n", g->m68k->data_registers, sizeof(g->m68k->data_registers));
    printf("m68k addr regs: %p (%zu)\n", g->m68k->address_registers, sizeof(g->m68k->data_registers));
}

void check_endianness()
{
    uint8_t* array = calloc(4, sizeof(uint8_t));
    array[0] = 0xAB;
    array[1] = 0xCD;
    array[2] = 0x12;
    array[3] = 0x34;
    for (int i = 0; i < 4; ++i)
        printf("byte %d: %#010X\n", i, array[i]);

    uint16_t* array_16 = (uint16_t*)array;
    for (int i = 0; i < 2; ++i)
        printf("word %d: %#010X\n", i, array_16[i]);
    for (int i = 0; i < 2; ++i)
        printf("word htonl %d: %#010X\n", i, htonl(array_16[i]));
    for (int i = 0; i < 2; ++i)
        printf("word htons %d: %#010X\n", i, htons(array_16[i]));

    uint32_t* array_32 = (uint32_t*)array;
    printf("long: %#010X\n", array_32[0]);
    printf("long htonl: %#010X\n", htonl(array_32[0]));

    free(array);
}

void check_decoding(Genesis* g, char* opcode_bin)
{
    uint16_t opcode = bin_parse(opcode_bin);
    g->memory[0] = MASK_BELOW(opcode, 8) >> 8;
    g->memory[1] = MASK_ABOVE_INC(opcode, 8);

    DecodedInstruction* decoded = genesis_decode(g, 0);
    printf("%s (%#03X | %#03X ) : %s\n", opcode_bin, g->memory[0], g->memory[1], decoded->mnemonics);
    // TODO free
}

void test_rom(Genesis* g, char* path)
{
    genesis_load_rom_file(g, path);
    genesis_initialize(g);

    bool running = true;
    while (running)
        running = genesis_run_frame(g);
}

void instr_callback(M68k* m)
{
    DecodedInstruction* d = m68k_decode(m, m->pc);
    printf("%#06X %s\n", m->pc, d->mnemonics);
    free(d);
}

int main(int argc, char **argv)
{
    Genesis* g = genesis_make();
#ifdef DEBUG
    g->m68k->instruction_callback = instr_callback;
#endif

    //check_alignment(g);
    //check_endianness();

    //check_decoding(g, "0100 0110 00 000010"); // NOT D2
    //check_decoding(g, "1100 100 010 000001"); // AND D4, D1
    //check_decoding(g, "0100111011 000011"); // JMP D3

    //getchar();

    if (argc < 2) {
      printf("No ROM specified, using default\n");
      test_rom(g, "../browser/test.bin");
    } else {
      test_rom(g, argv[1]);
    }

    genesis_free(g);

    return 0;
}
