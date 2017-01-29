#include <stdlib.h>

#include "instruction.h"
#include "m68k.h"
#include "operands.h"

int add(Instruction* i)
{
    uint64_t sum = GET(i->src) + GET(i->dst);
    SET(i->dst, sum);

    //TODO CARRY_SET(i->context, (int)result < (int)src);
    OVERFLOW_SET(i->context, sum > MAX_VALUE(i->size));
    ZERO_SET(i->context, sum == 0);
    NEGATIVE_SET(i->context, BIT(sum, i->size - 1));
    EXTENDED_SET(i->context, CARRY(i->context));

    return 0;
}

Instruction* gen_add(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "ADD", add);

    i->size = operand_size(FRAGMENT(opcode, 7, 6));

    Operand* reg = operand_make_data_register(FRAGMENT(opcode, 11, 9), i);
    Operand* ea = operand_make(FRAGMENT(opcode, 5, 0), i);

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

int clr(Instruction* i)
{
    SET(i->src, MASK_BELOW(GET(i->src), i->size));

    CARRY_SET(i->context, false);
    OVERFLOW_SET(i->context, false);
    ZERO_SET(i->context, true);
    NEGATIVE_SET(i->context, false);

    return 0;
}

Instruction* gen_clr(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "CLR", clr);
    i->size = operand_size(FRAGMENT(opcode, 7, 6));
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);
    return i;
}

int cmp(Instruction* i)
{
    uint32_t diff = GET(i->dst) - GET(i->src);

    CARRY_SET(i->context, false); // TODO
    OVERFLOW_SET(i->context, false); // TODO
    ZERO_SET(i->context, diff == 0);
    NEGATIVE_SET(i->context, BIT(diff, i->size - 1) == 1); // TODO not so sure, signed/unsigned arithmetic?

    return 0;
}

Instruction* gen_cmp(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "CMP", cmp);
    i->size = operand_size(FRAGMENT(opcode, 7, 6));
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);
    i->dst = operand_make_data_register(FRAGMENT(opcode, 11, 9), i);
    return i;
}

int ext(Instruction* i)
{
    int x = GET(i->src);

    uint32_t extended;
    switch (i->size) {
    case Word:
        extended = SIGN_EXTEND_B(x);
        break;
    case Long:
        extended = SIGN_EXTEND_W(x);
        break;
    }

    SET(i->src, extended);

    CARRY_SET(i->context, false);
    OVERFLOW_SET(i->context, false);
    ZERO_SET(i->context, extended == 0);
    NEGATIVE_SET(i->context, BIT(extended, i->size - 1) == 1);

    return 0;
}

Instruction* gen_ext(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "EXT", ext);
    i->size = operand_sign_extension(FRAGMENT(opcode, 8, 6));
    i->src = operand_make_data_register(FRAGMENT(opcode, 3, 0), i);
    return i;
}

int muls(Instruction* i)
{
    SET(i->src, GET(i->src) * GET(i->dst));

    CARRY_SET(i->context, false);
    OVERFLOW_SET(i->context, false); // TODO
    ZERO_SET(i->context, true); // TODO
    NEGATIVE_SET(i->context, false); // TODO
                                     // TODO EXT

    return 0;
}

Instruction* gen_muls(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "MULS", muls);
    i->size = operand_size(FRAGMENT(opcode, 7, 6));
    i->src = operand_make_data_register(FRAGMENT(opcode, 11, 9), i);
    i->dst = operand_make(FRAGMENT(opcode, 5, 0), i);
    return i;
}

int mulu(Instruction* i)
{
    SET(i->src, GET(i->src) * GET(i->dst));

    CARRY_SET(i->context, false);
    OVERFLOW_SET(i->context, false); // TODO
    ZERO_SET(i->context, true); // TODO
    NEGATIVE_SET(i->context, false); // TODO
                                     // TODO EXT

    return 0;
}

Instruction* gen_mulu(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "MULU", mulu);
    i->size = operand_size(FRAGMENT(opcode, 7, 6));
    i->src = operand_make_data_register(FRAGMENT(opcode, 11, 9), i);
    i->dst = operand_make(FRAGMENT(opcode, 5, 0), i);
    return i;
}
