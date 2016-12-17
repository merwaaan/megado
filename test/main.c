#include <genesis/genesis.h>
#include <m68k/instruction.h>
#include <m68k/m68k.h>

void check_alignment(Genesis* g)
{
    printf("m68k: %p (%d)\n", g->m68k, sizeof(g->m68k));
    printf("m68k data regs: %p (%d)\n", g->m68k->data_registers, sizeof(g->m68k->data_registers));
    printf("m68k addr regs: %p (%d)\n", g->m68k->address_registers, sizeof(g->m68k->data_registers));
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

int main()
{
    Genesis* g = genesis_make();

    check_alignment(g);
    check_decoding(g, "0100 0110 00 000010");
    check_decoding(g, "1100100010000001");
    check_decoding(g, "0100111011000011");

    genesis_free(g);

    return 0;
}
