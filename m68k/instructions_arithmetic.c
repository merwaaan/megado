#include <stdlib.h>

#include "instruction.h"
#include "instructions_arithmetic.h"

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
    i->name = "MULU";
    i->func = mulu;
    i->size = operand_size(FRAGMENT(opcode, 7, 6));
    i->src = operand_make_data(FRAGMENT(opcode, 11, 9), i);
    i->dst = operand_make(FRAGMENT(opcode, 5, 0), i);
    return i;
}
