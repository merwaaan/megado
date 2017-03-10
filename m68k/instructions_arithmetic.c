#include <stdlib.h>

#include "instruction.h"
#include "m68k.h"
#include "operands.h"

int add(Instruction* i)
{
    uint32_t b = FETCH_EA_AND_GET(i->src);
    uint32_t a = FETCH_EA_AND_GET(i->dst);
    SET(i->dst, a + b);

    uint32_t result = GET(i->dst);
    CARRY_SET(i->context, CHECK_CARRY_ADD(a, b, i->size));
    OVERFLOW_SET(i->context, CHECK_OVERFLOW_ADD(a, b, i->size));
    ZERO_SET(i->context, result == 0);
    NEGATIVE_SET(i->context, BIT(result, i->size - 1));
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

int adda(Instruction* i)
{
    uint32_t a = FETCH_EA_AND_GET(i->src);

    if (i->size == Word)
        SIGN_EXTEND_W(a);

    SET(i->dst, GET(i->dst) + a);

    return 0;
}

Instruction* gen_adda(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "ADDA", adda);
    i->size = BIT(opcode, 8) ? Long : Word;
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);
    i->dst = operand_make_address_register(FRAGMENT(opcode, 11, 9), i);
    return i;
}

Instruction* gen_addi(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "ADDI", add);
    i->size = operand_size(FRAGMENT(opcode, 7, 6));
    i->src = operand_make_immediate_value(i->size, i);
    i->dst = operand_make(FRAGMENT(opcode, 5, 0), i);
    return i;
}

int addq(Instruction* i)
{
    // Extract the quick value, 0 represents 8
    uint32_t quick = GET(i->src);
    if (quick == 0)
        quick = 8;

    uint32_t initial = FETCH_EA_AND_GET(i->dst);
    SET(i->dst, initial + quick);

    uint32_t result = GET(i->dst);
    CARRY_SET(i->context, CHECK_CARRY_ADD(initial, quick, i->size));
    OVERFLOW_SET(i->context, CHECK_OVERFLOW_ADD(initial, quick, i->size));
    ZERO_SET(i->context, result == 0);
    NEGATIVE_SET(i->context, BIT(result, i->size - 1));

    return 0;
}

Instruction* gen_addq(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "ADDQ", addq);
    i->size = operand_size(FRAGMENT(opcode, 7, 6));
    i->src = operand_make_value(FRAGMENT(opcode, 11, 9), i);
    i->dst = operand_make(FRAGMENT(opcode, 5, 0), i);
    return i;
}


Instruction* gen_addx(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "ADDX", not_implemented);
    return i;
}

int clr(Instruction* i)
{
    FETCH_EA_AND_SET(i->src, 0);

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
    uint32_t b = FETCH_EA_AND_GET(i->src);
    uint32_t a = FETCH_EA_AND_GET(i->dst);

    CARRY_SET(i->context, CHECK_CARRY_SUB(a, b, i->size));
    OVERFLOW_SET(i->context, CHECK_OVERFLOW_SUB(a, b, i->size));
    ZERO_SET(i->context, a == b);
    NEGATIVE_SET(i->context, BIT(a - b, i->size - 1));

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

int cmpa(Instruction* i)
{
    uint32_t b = FETCH_EA_AND_GET(i->src);
    if (i->size == Word)
        b = SIGN_EXTEND_W(b);

    uint32_t a = i->context->address_registers[i->dst->n];

    CARRY_SET(i->context, CHECK_CARRY_SUB(a, b, i->size));
    OVERFLOW_SET(i->context, CHECK_OVERFLOW_SUB(a, b, i->size));
    ZERO_SET(i->context, a == b);
    NEGATIVE_SET(i->context, BIT(a - b, i->size - 1));

    return 0;
}

Instruction* gen_cmpa(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "CMPA", cmpa);
    i->size = BIT(opcode, 8) ? Long : Word;
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);
    i->dst = operand_make_address_register(FRAGMENT(opcode, 11, 9), i);
    return i;
}

Instruction* gen_cmpi(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "CMPI", cmp);
    i->size = operand_size(FRAGMENT(opcode, 7, 6));
    i->src = operand_make_immediate_value(i->size, i);
    i->dst = operand_make(FRAGMENT(opcode, 5, 0), i);
    return i;
}

Instruction* gen_cmpm(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "SUB", not_implemented);
    return i;
}

int div_(Instruction* i)
{
    uint16_t a = FETCH_EA_AND_GET(i->src);
    uint16_t b = GET(i->dst);

    uint16_t quotient = a / b;
    uint16_t remainder = a % b;
    SET(i->dst, remainder << 16 | quotient);

    CARRY_SET(i->context, false);
    OVERFLOW_SET(i->context, false); // TODO
    ZERO_SET(i->context, quotient == 0);
    NEGATIVE_SET(i->context, BIT(quotient, 15));

    return 0;
}

Instruction* gen_divs(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "DIVS", div_);
    i->size = Long;
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);
    i->dst = operand_make_data_register(FRAGMENT(opcode, 11, 9), i);
    return i;
}

Instruction* gen_divu(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "DIVU", div_);
    i->size = Long;
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

int mul(Instruction* i)
{
    SET(i->dst, FETCH_EA_AND_GET(i->src) * GET(i->dst));

    uint32_t result = GET(i->dst);
    CARRY_SET(i->context, false);
    OVERFLOW_SET(i->context, false); // TODO
    ZERO_SET(i->context, result == 0);
    NEGATIVE_SET(i->context, BIT(result, i->size - 1));

    return 0;
}

Instruction* gen_muls(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "MULS", mul);
    i->size = Long;
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);
    i->dst = operand_make_data_register(FRAGMENT(opcode, 11, 9), i);
    return i;
}

Instruction* gen_mulu(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "MULU", mul);
    i->size = Long;
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);
    i->dst = operand_make_data_register(FRAGMENT(opcode, 11, 9), i);
    return i;
}

int neg(Instruction* i)
{
    uint32_t value = FETCH_EA_AND_GET(i->dst);
    SET(i->dst, 0 - value);

    uint32_t result = GET(i->dst);
    CARRY_SET(i->context, CHECK_CARRY_SUB(0, value, i->size));
    OVERFLOW_SET(i->context, CHECK_OVERFLOW_SUB(0, value, i->size));
    ZERO_SET(i->context, result == 0);
    NEGATIVE_SET(i->context, BIT(result, i->size - 1));
    EXTENDED_SET(i->context, CARRY(i->context));

    return 0;
}

Instruction* gen_neg(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "NEG", neg);
    i->size = operand_size(FRAGMENT(opcode, 7, 6));
    i->dst = operand_make(FRAGMENT(opcode, 5, 0), i);
    return i;
}

Instruction* gen_negx(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "NEGX", not_implemented);
    return i;
}

int sub(Instruction* i)
{
    uint32_t b = FETCH_EA_AND_GET(i->src);
    uint32_t a = FETCH_EA_AND_GET(i->dst);
    SET(i->dst, a - b);

    CARRY_SET(i->context, CHECK_CARRY_SUB(a, b, i->size));
    OVERFLOW_SET(i->context, CHECK_OVERFLOW_SUB(a, b, i->size));
    ZERO_SET(i->context, a == b);
    NEGATIVE_SET(i->context, BIT(a - b, i->size - 1));
    EXTENDED_SET(i->context, CARRY(i->context));

    return 0;
}

Instruction* gen_sub(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "SUB", sub);
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

int suba(Instruction* i)
{
    uint32_t a = FETCH_EA_AND_GET(i->src);

    if (i->size == Word)
        SIGN_EXTEND_W(a);

    SET(i->dst, GET(i->dst) - a);

    return 0;
}

Instruction* gen_suba(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "SUBA", suba);
    i->size = BIT(opcode, 8) ? Long : Word;
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);
    i->dst = operand_make_address_register(FRAGMENT(opcode, 11, 9), i);
    return i;
}

Instruction* gen_subi(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "SUBI", sub);
    i->size = operand_size(FRAGMENT(opcode, 7, 6));
    i->src = operand_make_immediate_value(i->size, i);
    i->dst = operand_make(FRAGMENT(opcode, 5, 0), i);
    return i;
}

int subq(Instruction* i)
{
    // Extract the quick value, 0 represents 8
    uint32_t quick = GET(i->src);
    if (quick == 0)
        quick = 8;

    uint32_t initial = FETCH_EA_AND_GET(i->dst);
    SET(i->dst, initial - quick);

    CARRY_SET(i->context, CHECK_CARRY_SUB(initial, quick, i->size));
    OVERFLOW_SET(i->context, CHECK_OVERFLOW_SUB(initial, quick, i->size));
    ZERO_SET(i->context, initial == quick);
    NEGATIVE_SET(i->context, BIT(initial - quick, i->size - 1));
    EXTENDED_SET(i->context, CARRY(i->context));

    return 0;
}

Instruction* gen_subq(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "SUBQ", subq);
    i->size = operand_size(FRAGMENT(opcode, 7, 6));
    i->src = operand_make_value(FRAGMENT(opcode, 11, 9), i);
    i->dst = operand_make(FRAGMENT(opcode, 5, 0), i);
    return i;
}

Instruction* gen_subx(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "SUBX", not_implemented);
    return i;
}
