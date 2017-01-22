#include <stdbool.h>
#include <stdlib.h>

#include "bit_utils.h"
#include "instruction.h"
#include "m68k.h"
#include "operands.h"

Instruction* gen_bit_instruction(uint16_t opcode, M68k* m, char* name, InstructionFunc func)
{
    Instruction* i = instruction_make(m, name, func);
    i->size = Long;
    i->src = operand_make_data_register(FRAGMENT(opcode, 11, 9), i);
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
    int bit = GET(i->src) % 32;
    int set = BIT(GET(i->dst), bit);

    ZERO_SET(i->context, !set);
}

Instruction* gen_btst(uint16_t opcode, M68k* m)
{
    return gen_bit_instruction(opcode, m, "BTST", btst);
}

void btst_imm(Instruction* i)
{
    int bit = m68k_read_b(i->context, i->context->pc + 3) % 32;
    int set = BIT(GET(i->dst), bit);

    ZERO_SET(i->context, !set);
}

Instruction* gen_btst_imm(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "BTST", btst_imm);
    i->size = Long;
    i->dst = operand_make(FRAGMENT(opcode, 5, 0), i); // TODO use operand for immediate data so that it gets properly disassembled and the same implem can be used
    return i;
}