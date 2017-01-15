#include <stdbool.h>
#include <stdlib.h>

#include "bit_utils.h"
#include "instruction.h"
#include "m68k.h"
#include "operands.h"

Instruction* gen_bit_instruction(uint16_t opcode, M68k* m, char* name, InstructionFunc func)
{
    Instruction* i = calloc(1, sizeof(Instruction));
    i->context = m;
    i->name = name;
    i->func = func;

    i->size = operand_size(FRAGMENT(opcode, 7, 6));

    i->src = operand_make_data(FRAGMENT(opcode, 11, 9), i);
    i->dst = operand_make(FRAGMENT(opcode, 5, 0), i);

    return i;
}

void bchg(Instruction* i)
{
    int bit = GET(i->src); // TODO
    int initial = GET(i->dst);
    SET(i->dst, BIT_CHG(initial, bit, !BIT(initial, bit)));

    ZERO_SET(i->context, BIT(initial, bit) == 0);
}

Instruction* gen_bchg(uint16_t opcode, M68k* m)
{
    return gen_bit_instruction(opcode, m, "BCHG", bchg);
}

void bclr(Instruction* i)
{
    int initial = GET(i->dst);
    int bit = GET(i->src);
    SET(i->dst, BIT_CLR(initial, bit));

    ZERO_SET(i->context, BIT(initial, bit) == 0);
}

Instruction* gen_bclr(uint16_t opcode, M68k* m)
{
    return gen_bit_instruction(opcode, m, "BCLR", bclr);
}

void bset(Instruction* i)
{
    int initial = GET(i->dst);
    int bit = GET(i->src);
    SET(i->dst, BIT_SET(initial, bit));

    ZERO_SET(i->context, BIT(initial, bit) == 0);
}

Instruction* gen_bset(uint16_t opcode, M68k* m)
{
    return gen_bit_instruction(opcode, m, "BSET", bset);
}

void btst(Instruction* i)
{
    int bit = GET(i->src);
    int test = BIT(GET(i->dst), bit) == 0;
    ZERO_SET(i->context, test);
}

Instruction* gen_btst(uint16_t opcode, M68k* m)
{
    return gen_bit_instruction(opcode, m, "BTST", btst);
}