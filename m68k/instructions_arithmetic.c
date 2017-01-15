#include <stdlib.h>

#include "instruction.h"
#include "instructions_arithmetic.h"

void add(Instruction* i)
{
    uint64_t src = GET(i->src);
    uint64_t dst = GET(i->dst);
    uint64_t sum = src + dst;

    uint32_t result = MASK_ABOVE_INC(sum, i->size);
    SET(i->src, MASK_BELOW(dst, i->size) | result);

    CARRY_SET(i->context, (int)result < (int)src);
    OVERFLOW_SET(i->context, sum > MAX_VALUE(i->size));
    ZERO_SET(i->context, result == 0);
    NEGATIVE_SET(i->context, BIT(result, i->size - 1));
    EXTENDED_SET(i->context, CARRY(i->context));
}

Instruction* gen_add(uint16_t opcode, M68k* m)
{
    Instruction* i = calloc(1, sizeof(Instruction));
    i->context = m;
    i->name = "ADD";
    i->func = add;

    i->size = operand_size(FRAGMENT(opcode, 7, 6));

    int reg = operand_make_data(FRAGMENT(opcode, 11, 9), i);
    int ea = operand_make(FRAGMENT(opcode, 5, 0), i);

    int direction = BIT(opcode, 8);
    if (direction == 0)
    {
        i->src = ea;
        i->dst = reg;
    }
    else
    {
        i->src = reg;
        i->dst = ea;
    }

    return i;
}

void clr(Instruction* i)
{
    SET(i->src, MASK_BELOW(GET(i->src), i->size));

    CARRY_SET(i->context, false);
    OVERFLOW_SET(i->context, false);
    ZERO_SET(i->context, true);
    NEGATIVE_SET(i->context, false);
}

Instruction* gen_clr(uint16_t opcode, M68k* m)
{
    Instruction* i = calloc(1, sizeof(Instruction));
    i->context = m;
    i->name = "CLR";
    i->func = clr;
    i->size = operand_size(FRAGMENT(opcode, 7, 6));
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);
    return i;
}

void ext(Instruction* i)
{
    int x = GET(i->src);

    uint32_t extended;
    switch (i->size) {
    case 8:
        extended = SIGN_EXTEND_B(x);
        break;
    case 16:
        extended = SIGN_EXTEND_W(x);
        break;
    }

    SET(i->src, extended);

    CARRY_SET(i->context, false);
    OVERFLOW_SET(i->context, false);
    ZERO_SET(i->context, extended == 0);
    NEGATIVE_SET(i->context, BIT(extended, i->size - 1) == 1);
}

Instruction* gen_ext(uint16_t opcode, M68k* m)
{
    Instruction* i = calloc(1, sizeof(Instruction));
    i->context = m;
    i->name = "EXT";
    i->func = ext;
    i->size = operand_sign_extension(FRAGMENT(opcode, 8, 6));
    i->src = operand_make_data(FRAGMENT(opcode, 3, 0), i);
    return i;
}

void muls(Instruction* i)
{
    SET(i->src, GET(i->src) * GET(i->dst));

    CARRY_SET(i->context, false);
    OVERFLOW_SET(i->context, false); // TODO
    ZERO_SET(i->context, true); // TODO
    NEGATIVE_SET(i->context, false); // TODO
                                     // TODO EXT
}

Instruction* gen_muls(uint16_t opcode, M68k* m)
{
    Instruction* i = calloc(1, sizeof(Instruction));
    i->context = m;
    i->name = "MULS";
    i->func = muls;
    i->size = operand_size(FRAGMENT(opcode, 7, 6));
    i->src = operand_make_data(FRAGMENT(opcode, 11, 9), i);
    i->dst = operand_make(FRAGMENT(opcode, 5, 0), i);
    return i;
}

void mulu(Instruction* i)
{
    SET(i->src, GET(i->src) * GET(i->dst));

    CARRY_SET(i->context, false);
    OVERFLOW_SET(i->context, false); // TODO
    ZERO_SET(i->context, true); // TODO
    NEGATIVE_SET(i->context, false); // TODO
                                     // TODO EXT
}

Instruction* gen_mulu(uint16_t opcode, M68k* m)
{
    Instruction* i = calloc(1, sizeof(Instruction));
    i->context = m;
    i->name = "MULU";
    i->func = mulu;
    i->size = operand_size(FRAGMENT(opcode, 7, 6));
    i->src = operand_make_data(FRAGMENT(opcode, 11, 9), i);
    i->dst = operand_make(FRAGMENT(opcode, 5, 0), i);
    return i;
}
