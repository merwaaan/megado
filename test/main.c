#include <megado/genesis.h>
#include <megado/m68k/instruction.h>
#include <megado/m68k/m68k.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

void run(Genesis* g, char* path)
{
    genesis_load_rom_file(g, path);
    genesis_initialize(g);

    while (g->status != Status_Quitting)
        genesis_update(g);
}

int main(int argc, char **argv)
{
    // Generate the instruction table
    opcode_table = calloc(0x10000, sizeof(Instruction*));
    for (int opcode = 0; opcode < 0x10000; ++opcode)
        opcode_table[opcode] = instruction_generate(opcode);

    Genesis* g = genesis_make();

    if (argc < 2) {
        printf("No ROM specified, using default\n");
        run(g, "../browser/test.bin");
    }
    else {
        run(g, argv[1]);
    }

    genesis_free(g);

    for (int opcode = 0; opcode < 0x10000; ++opcode)
        opcode_table[opcode] = instruction_generate(opcode);
    free(opcode_table);

    return 0;
}
