#include <stdlib.h>

#include "cycles.h"
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

    if (instruction_is_valid(i, true, true))
        i->base_cycles = cycles_standard_instruction(i, 8, 4, 8);

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

    if (instruction_is_valid(i, true, true))
        i->base_cycles = i->size == Long ?
        cycles_immediate_instruction(i, 16, 0, 20) :
        cycles_immediate_instruction(i, 8, 0, 12);

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

    if (instruction_is_valid(i, true, true))
        i->base_cycles = i->size == Long ?
        cycles_immediate_instruction(i, 8, 8, 12) :
        cycles_immediate_instruction(i, 4, 8, 8);

    return i;
}

int addx(Instruction* i)
{
    uint32_t b = FETCH_EA_AND_GET(i->src);
    uint32_t a = FETCH_EA_AND_GET(i->dst);
    bool extended = EXTENDED(i->context);
    SET(i->dst, a + b + extended);

    uint32_t result = GET(i->dst);
    CARRY_SET(i->context, CHECK_CARRY_ADD(a, b + extended, i->size)); // TODO does this work?
    OVERFLOW_SET(i->context, CHECK_OVERFLOW_ADD(a, b + extended, i->size));
    ZERO_SET(i->context, result == 0);
    NEGATIVE_SET(i->context, BIT(result, i->size - 1));
    EXTENDED_SET(i->context, CARRY(i->context));

    return 0;
}

Instruction* gen_addx(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "ADDX", addx);
    i->size = operand_size(FRAGMENT(opcode, 7, 6));

    bool data_register = BIT(opcode, 3);
    if (data_register)
    {
        i->src = operand_make_address_register_indirect_predec(FRAGMENT(opcode, 2, 0), i);
        i->dst = operand_make_address_register_indirect_predec(FRAGMENT(opcode, 11, 9), i);
    }
    else
    {
        i->src = operand_make_data_register(FRAGMENT(opcode, 2, 0), i);
        i->dst = operand_make_data_register(FRAGMENT(opcode, 11, 9), i);
    }

    return i;
}

int clr(Instruction* i)
{
    FETCH_EA_AND_SET(i->dst, 0);

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
    i->dst = operand_make(FRAGMENT(opcode, 5, 0), i);

    if (instruction_is_valid(i, false, true))
        i->base_cycles = i->size == Long ?
            cycles_single_operand_instruction(i, 6, 12) :
            cycles_single_operand_instruction(i, 4, 8);

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

    if (instruction_is_valid(i, true, true))
        i->base_cycles = i->size == Long ?
            cycles_standard_instruction(i, 6, 4, 0) :
            cycles_standard_instruction(i, 6, 6, 0);

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

    if (instruction_is_valid(i, true, true))
        i->base_cycles = i->size == Long ?
        cycles_immediate_instruction(i, 14, 0, 12) :
        cycles_immediate_instruction(i, 8, 0, 8);

    return i;
}

Instruction* gen_cmpm(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "CMPM", cmp);
    i->size = operand_size(FRAGMENT(opcode, 7, 6));
    i->src = operand_make_address_register_indirect_postinc(FRAGMENT(opcode, 2, 0), i);
    i->dst = operand_make_address_register_indirect_postinc(FRAGMENT(opcode, 11, 9), i);
    return i;
}

int divu(Instruction* i)
{
    uint16_t a = FETCH_EA_AND_GET(i->src);
    uint32_t b = i->context->data_registers[i->dst->n];

    if (a == 0) {
        // TODO: trap the divide by zero
        // operands are unaffected, flags are undefined
    }
    else {
        uint32_t quotient = b / a;
        uint16_t remainder = b % a;
        i->context->data_registers[i->dst->n] = remainder << 16 | (quotient & 0xFFFF);

        CARRY_SET(i->context, false);
        OVERFLOW_SET(i->context, (quotient & 0xFFFF0000) > 0);
        ZERO_SET(i->context, quotient == 0);
        NEGATIVE_SET(i->context, BIT(quotient, 15));
    }

    return 144; // TODO: should add address calculation time
}

int divs(Instruction* i)
{
    // Same as DIVU, but assuming signed operands
    int16_t a = FETCH_EA_AND_GET(i->src);
    int32_t b = i->context->data_registers[i->dst->n];

    if (a == 0) {
        // TODO: trap the divide by zero
        // operands are unaffected, flags are undefined
    }
    else {
        int32_t quotient = b / a;
        int16_t remainder = b % a;
        i->context->data_registers[i->dst->n] = remainder << 16 | (quotient & 0xFFFF);

        CARRY_SET(i->context, false);
        OVERFLOW_SET(i->context, (quotient & 0xFFFF0000) > 0);
        ZERO_SET(i->context, quotient == 0);
        NEGATIVE_SET(i->context, BIT(quotient, 15));
    }

    return 162; // TODO: should add address calculation time
}

Instruction* gen_divs(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "DIVS", divs);
    i->size = Word;
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);
    i->dst = operand_make_data_register(FRAGMENT(opcode, 11, 9), i);

    if (instruction_is_valid(i, true, true))
        i->base_cycles = cycles_standard_instruction(i, 0, 158, 0);

    return i;
}

Instruction* gen_divu(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "DIVU", divu);
    i->size = Word;
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);
    i->dst = operand_make_data_register(FRAGMENT(opcode, 11, 9), i);

    if (instruction_is_valid(i, true, true))
        i->base_cycles = cycles_standard_instruction(i, 0, 140, 0);

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
    i->base_cycles = 4;
    return i;
}

int mul(Instruction* i)
{
    i->context->data_registers[i->dst->n] = FETCH_EA_AND_GET(i->src) * i->context->data_registers[i->dst->n];

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
    i->size = Word;
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);
    i->dst = operand_make_data_register(FRAGMENT(opcode, 11, 9), i);

    if (instruction_is_valid(i, true, true))
        i->base_cycles = cycles_standard_instruction(i, 0, 70, 0);

    return i;
}

Instruction* gen_mulu(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "MULU", mul);
    i->size = Word;
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);
    i->dst = operand_make_data_register(FRAGMENT(opcode, 11, 9), i);

    if (instruction_is_valid(i, true, true))
        i->base_cycles = cycles_standard_instruction(i, 0, 70, 0);

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

    if (instruction_is_valid(i, false, true))
        i->base_cycles = i->size == Long ?
            cycles_single_operand_instruction(i, 6, 12) :
            cycles_single_operand_instruction(i, 4, 8);

    return i;
}

Instruction* gen_negx(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "NEGX", not_implemented);

    if (instruction_is_valid(i, false, true))
        i->base_cycles = i->size == Long ?
            cycles_single_operand_instruction(i, 6, 12) :
            cycles_single_operand_instruction(i, 4, 8);

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

    if (instruction_is_valid(i, true, true))
        i->base_cycles = i->size == Long ?
            cycles_standard_instruction(i, 8, 4, 8) :
            cycles_standard_instruction(i, 6, 6, 12);

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

    if (instruction_is_valid(i, true, true))
        i->base_cycles = i->size == Long ?
        cycles_immediate_instruction(i, 16, 0, 20) :
        cycles_immediate_instruction(i, 8, 0, 12);

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

    if (instruction_is_valid(i, true, true))
        i->base_cycles = i->size == Long ?
        cycles_immediate_instruction(i, 8, 8, 12) :
        cycles_immediate_instruction(i, 4, 8, 8);

    return i;
}

Instruction* gen_subx(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "SUBX", not_implemented);
    return i;
}

int tas(Instruction* i)
{
    uint32_t value = FETCH_EA_AND_GET(i->src);

    CARRY_SET(i->context, false);
    OVERFLOW_SET(i->context, false);
    ZERO_SET(i->context, value == 0);
    NEGATIVE_SET(i->context, BIT(value, 7));

    return 0;
}

Instruction* gen_tas(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "TAS", tas);
    i->size = Byte;
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);
    return i;
}
