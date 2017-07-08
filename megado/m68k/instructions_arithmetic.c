#include <stdio.h>
#include <stdlib.h>

#include "cycles.h"
#include "instruction.h"
#include "m68k.h"
#include "operands.h"

uint8_t add(Instruction* i, M68k* ctx)
{
    uint32_t b = FETCH_EA_AND_GET(i->src, ctx);
    uint32_t a = FETCH_EA_AND_GET(i->dst, ctx);
    SET(i->dst, ctx, a + b);

    uint32_t result = GET(i->dst, ctx);
    CARRY_SET(ctx, CHECK_CARRY_ADD(a, b, i->size));
    OVERFLOW_SET(ctx, CHECK_OVERFLOW_ADD(a, b, i->size));
    ZERO_SET(ctx, result == 0);
    NEGATIVE_SET(ctx, BIT(result, i->size - 1));
    EXTENDED_SET(ctx, CARRY(ctx));

    return 0;
}

Instruction* gen_add(uint16_t opcode)
{
    Instruction* i = instruction_make("ADD", add);

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

    i->base_cycles = i->size == Long ?
        cycles_standard_instruction(i, 8, 4, 8) :
        cycles_standard_instruction(i, 6, 6, 12); // TODO increased to eight?

    return i;
}

uint8_t adda(Instruction* i, M68k* ctx)
{
    uint32_t a = FETCH_EA_AND_GET(i->src, ctx);

    if (i->size == Word)
        SIGN_EXTEND_W(a);

    SET(i->dst, ctx, GET(i->dst, ctx) + a);

    return 0;
}

Instruction* gen_adda(uint16_t opcode)
{
    Instruction* i = instruction_make("ADDA", adda);
    i->size = BIT(opcode, 8) ? Long : Word;
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);
    i->dst = operand_make_address_register(FRAGMENT(opcode, 11, 9), i);

    i->base_cycles = i->size == Long ?
        cycles_standard_instruction(i, 8, 4, 8) :
        cycles_standard_instruction(i, 6, 6, 12); // TODO increased to eight?

    return i;
}

Instruction* gen_addi(uint16_t opcode)
{
    Instruction* i = instruction_make("ADDI", add);
    i->size = operand_size(FRAGMENT(opcode, 7, 6));
    i->src = operand_make_immediate_value(i->size, i);
    i->dst = operand_make(FRAGMENT(opcode, 5, 0), i);

    i->base_cycles = i->size == Long ?
        cycles_immediate_instruction(i, 16, 0, 20) :
        cycles_immediate_instruction(i, 8, 0, 12);

    return i;
}

uint8_t addq(Instruction* i, M68k* ctx)
{
    // Extract the quick value, 0 represents 8
    uint32_t quick = GET(i->src, ctx);
    if (quick == 0)
        quick = 8;

    uint32_t initial = FETCH_EA_AND_GET(i->dst, ctx);
    SET(i->dst, ctx, initial + quick);

    uint32_t result = GET(i->dst, ctx);
    CARRY_SET(ctx, CHECK_CARRY_ADD(initial, quick, i->size));
    OVERFLOW_SET(ctx, CHECK_OVERFLOW_ADD(initial, quick, i->size));
    ZERO_SET(ctx, result == 0);
    NEGATIVE_SET(ctx, BIT(result, i->size - 1));

    return 0;
}

Instruction* gen_addq(uint16_t opcode)
{
    Instruction* i = instruction_make("ADDQ", addq);
    i->size = operand_size(FRAGMENT(opcode, 7, 6));
    i->src = operand_make_value(FRAGMENT(opcode, 11, 9), i);
    i->dst = operand_make(FRAGMENT(opcode, 5, 0), i);

    i->base_cycles = i->size == Long ?
        cycles_immediate_instruction(i, 8, 8, 12) :
        cycles_immediate_instruction(i, 4, 4, 8);

    return i;
}

uint8_t addx(Instruction* i, M68k* ctx)
{
    uint32_t b = FETCH_EA_AND_GET(i->src, ctx);
    uint32_t a = FETCH_EA_AND_GET(i->dst, ctx);
    bool extended = EXTENDED(ctx);
    SET(i->dst, ctx, a + b + extended);

    uint32_t result = GET(i->dst, ctx);
    CARRY_SET(ctx, CHECK_CARRY_ADD(a, b + extended, i->size)); // TODO does this work?
    OVERFLOW_SET(ctx, CHECK_OVERFLOW_ADD(a, b + extended, i->size));
    ZERO_SET(ctx, result == 0);
    NEGATIVE_SET(ctx, BIT(result, i->size - 1));
    EXTENDED_SET(ctx, CARRY(ctx));

    return 0;
}

Instruction* gen_addx(uint16_t opcode)
{
    Instruction* i = instruction_make("ADDX", addx);
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

uint8_t clr(Instruction* i, M68k* ctx)
{
    FETCH_EA_AND_SET(i->src, ctx, 0);

    CARRY_SET(ctx, false);
    OVERFLOW_SET(ctx, false);
    ZERO_SET(ctx, true);
    NEGATIVE_SET(ctx, false);

    return 0;
}

Instruction* gen_clr(uint16_t opcode)
{
    Instruction* i = instruction_make("CLR", clr);
    i->size = operand_size(FRAGMENT(opcode, 7, 6));
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);

    i->base_cycles = i->size == Long ?
        cycles_single_operand_instruction(i, 6, 12) :
        cycles_single_operand_instruction(i, 4, 8);

    return i;
}

uint8_t cmp(Instruction* i, M68k* ctx)
{
    uint32_t b = FETCH_EA_AND_GET(i->src, ctx);
    uint32_t a = FETCH_EA_AND_GET(i->dst, ctx);

    CARRY_SET(ctx, CHECK_CARRY_SUB(a, b, i->size));
    OVERFLOW_SET(ctx, CHECK_OVERFLOW_SUB(a, b, i->size));
    ZERO_SET(ctx, a == b);
    NEGATIVE_SET(ctx, BIT(a - b, i->size - 1));

    return 0;
}

Instruction* gen_cmp(uint16_t opcode)
{
    Instruction* i = instruction_make("CMP", cmp);
    i->size = operand_size(FRAGMENT(opcode, 7, 6));
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);
    i->dst = operand_make_data_register(FRAGMENT(opcode, 11, 9), i);

    i->base_cycles = i->size == Long ?
        cycles_standard_instruction(i, 6, 6, 0) :
        cycles_standard_instruction(i, 6, 4, 0);

    return i;
}

uint8_t cmpa(Instruction* i, M68k* ctx)
{
    uint32_t b = FETCH_EA_AND_GET(i->src, ctx);
    if (i->size == Word)
        b = SIGN_EXTEND_W(b);

    uint32_t a = ctx->address_registers[i->dst->n];

    CARRY_SET(ctx, CHECK_CARRY_SUB(a, b, i->size));
    OVERFLOW_SET(ctx, CHECK_OVERFLOW_SUB(a, b, i->size));
    ZERO_SET(ctx, a == b);
    NEGATIVE_SET(ctx, BIT(a - b, i->size - 1));

    return 0;
}

Instruction* gen_cmpa(uint16_t opcode)
{
    Instruction* i = instruction_make("CMPA", cmpa);
    i->size = BIT(opcode, 8) ? Long : Word;
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);
    i->dst = operand_make_address_register(FRAGMENT(opcode, 11, 9), i);

    i->base_cycles = i->size == Long ?
        cycles_standard_instruction(i, 6, 6, 0) :
        cycles_standard_instruction(i, 6, 4, 0);

    return i;
}

Instruction* gen_cmpi(uint16_t opcode)
{
    Instruction* i = instruction_make("CMPI", cmp);
    i->size = operand_size(FRAGMENT(opcode, 7, 6));
    i->src = operand_make_immediate_value(i->size, i);
    i->dst = operand_make(FRAGMENT(opcode, 5, 0), i);

    i->base_cycles = i->size == Long ?
        cycles_immediate_instruction(i, 14, 0, 12) :
        cycles_immediate_instruction(i, 8, 0, 8);

    return i;
}

Instruction* gen_cmpm(uint16_t opcode)
{
    Instruction* i = instruction_make("CMPM", cmp);
    i->size = operand_size(FRAGMENT(opcode, 7, 6));
    i->src = operand_make_address_register_indirect_postinc(FRAGMENT(opcode, 2, 0), i);
    i->dst = operand_make_address_register_indirect_postinc(FRAGMENT(opcode, 11, 9), i);
    return i;
}

uint8_t divu(Instruction* i, M68k* ctx)
{
    uint16_t a = FETCH_EA_AND_GET(i->src, ctx);
    uint32_t b = ctx->data_registers[i->dst->n];

    if (a == 0) {
        // TODO: trap the divide by zero
        // operands are unaffected, flags are undefined
    }
    else {
        uint32_t quotient = b / a;
        uint16_t remainder = b % a;
        ctx->data_registers[i->dst->n] = remainder << 16 | (quotient & 0xFFFF);

        CARRY_SET(ctx, false);
        OVERFLOW_SET(ctx, (quotient & 0xFFFF0000) > 0);
        ZERO_SET(ctx, quotient == 0);
        NEGATIVE_SET(ctx, BIT(quotient, 15));
    }

    return 144; // TODO: should add address calculation time
}

uint8_t divs(Instruction* i, M68k* ctx)
{
    // Same as DIVU, but assuming signed operands
    int16_t a = FETCH_EA_AND_GET(i->src, ctx);
    int32_t b = ctx->data_registers[i->dst->n];

    if (a == 0) {
        // TODO: trap the divide by zero
        // operands are unaffected, flags are undefined
    }
    else {
        int32_t quotient = b / a;
        uint32_t remainder = b % a;
        ctx->data_registers[i->dst->n] = remainder << 16 | (quotient & 0xFFFF);

        CARRY_SET(ctx, false);
        OVERFLOW_SET(ctx, (quotient & 0xFFFF0000) > 0);
        ZERO_SET(ctx, quotient == 0);
        NEGATIVE_SET(ctx, BIT(quotient, 15));
    }

    return 162; // TODO: should add address calculation time
}

Instruction* gen_divs(uint16_t opcode)
{
    Instruction* i = instruction_make("DIVS", divs);
    i->size = Word;
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);
    i->dst = operand_make_data_register(FRAGMENT(opcode, 11, 9), i);
    i->base_cycles = cycles_standard_instruction(i, 0, 158, 0);
    return i;
}

Instruction* gen_divu(uint16_t opcode)
{
    Instruction* i = instruction_make("DIVU", divu);
    i->size = Word;
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);
    i->dst = operand_make_data_register(FRAGMENT(opcode, 11, 9), i);
    i->base_cycles = cycles_standard_instruction(i, 0, 140, 0);
    return i;
}

uint8_t ext(Instruction* i, M68k* ctx)
{
    int x = GET(i->src, ctx);

    uint32_t extended;
    switch (i->size) {
    case Word:
        extended = SIGN_EXTEND_B(x);
        break;
    case Long:
        extended = SIGN_EXTEND_W(x);
        break;
    default:
        fprintf(stderr, "Invalid size %x in gen_ext\n", i->size);
        exit(1);
    }

    SET(i->src, ctx, extended);

    CARRY_SET(ctx, false);
    OVERFLOW_SET(ctx, false);
    ZERO_SET(ctx, extended == 0);
    NEGATIVE_SET(ctx, BIT(extended, i->size - 1) == 1);

    return 0;
}

Instruction* gen_ext(uint16_t opcode)
{
    Instruction* i = instruction_make("EXT", ext);
    i->size = operand_sign_extension(FRAGMENT(opcode, 8, 6));
    i->src = operand_make_data_register(FRAGMENT(opcode, 3, 0), i);
    i->base_cycles = 4;
    return i;
}

uint8_t mul(Instruction* i, M68k* ctx)
{
    ctx->data_registers[i->dst->n] = FETCH_EA_AND_GET(i->src, ctx) * ctx->data_registers[i->dst->n];

    uint32_t result = GET(i->dst, ctx);
    CARRY_SET(ctx, false);
    OVERFLOW_SET(ctx, false); // TODO
    ZERO_SET(ctx, result == 0);
    NEGATIVE_SET(ctx, BIT(result, i->size - 1));

    return 0;
}

Instruction* gen_muls(uint16_t opcode)
{
    Instruction* i = instruction_make("MULS", mul);
    i->size = Word;
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);
    i->dst = operand_make_data_register(FRAGMENT(opcode, 11, 9), i);
    i->base_cycles = cycles_standard_instruction(i, 0, 70, 0);
    return i;
}

Instruction* gen_mulu(uint16_t opcode)
{
    Instruction* i = instruction_make("MULU", mul);
    i->size = Word;
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);
    i->dst = operand_make_data_register(FRAGMENT(opcode, 11, 9), i);
    i->base_cycles = cycles_standard_instruction(i, 0, 70, 0);
    return i;
}

uint8_t neg(Instruction* i, M68k* ctx)
{
    uint32_t value = FETCH_EA_AND_GET(i->src, ctx);
    SET(i->src, ctx, 0 - value);

    uint32_t result = GET(i->src, ctx);
    CARRY_SET(ctx, CHECK_CARRY_SUB(0, value, i->size));
    OVERFLOW_SET(ctx, CHECK_OVERFLOW_SUB(0, value, i->size));
    ZERO_SET(ctx, result == 0);
    NEGATIVE_SET(ctx, BIT(result, i->size - 1));
    EXTENDED_SET(ctx, CARRY(ctx));

    return 0;
}

Instruction* gen_neg(uint16_t opcode)
{
    Instruction* i = instruction_make("NEG", neg);
    i->size = operand_size(FRAGMENT(opcode, 7, 6));
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);

    i->base_cycles = i->size == Long ?
        cycles_single_operand_instruction(i, 6, 12) :
        cycles_single_operand_instruction(i, 4, 8);

    return i;
}

Instruction* gen_negx(uint16_t opcode)
{
    Instruction* i = instruction_make("NEGX", not_implemented);

    /*i->base_cycles = i->size == Long ?
        cycles_single_operand_instruction(i, 6, 12) :
        cycles_single_operand_instruction(i, 4, 8);
        */
    return i;
}

uint8_t sub(Instruction* i, M68k* ctx)
{
    uint32_t b = FETCH_EA_AND_GET(i->src, ctx);
    uint32_t a = FETCH_EA_AND_GET(i->dst, ctx);
    SET(i->dst, ctx, a - b);

    CARRY_SET(ctx, CHECK_CARRY_SUB(a, b, i->size));
    OVERFLOW_SET(ctx, CHECK_OVERFLOW_SUB(a, b, i->size));
    ZERO_SET(ctx, a == b);
    NEGATIVE_SET(ctx, BIT(a - b, i->size - 1));
    EXTENDED_SET(ctx, CARRY(ctx));

    return 0;
}

Instruction* gen_sub(uint16_t opcode)
{
    Instruction* i = instruction_make("SUB", sub);
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

    i->base_cycles = i->size == Long ?
        cycles_standard_instruction(i, 8, 4, 8) :
        cycles_standard_instruction(i, 6, 6, 12); // TODO increase to eight?!

    return i;
}

uint8_t suba(Instruction* i, M68k* ctx)
{
    uint32_t a = FETCH_EA_AND_GET(i->src, ctx);

    if (i->size == Word)
        SIGN_EXTEND_W(a);

    SET(i->dst, ctx, GET(i->dst, ctx) - a);

    return 0;
}

Instruction* gen_suba(uint16_t opcode)
{
    Instruction* i = instruction_make("SUBA", suba);
    i->size = BIT(opcode, 8) ? Long : Word;
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);
    i->dst = operand_make_address_register(FRAGMENT(opcode, 11, 9), i);
    return i;
}

Instruction* gen_subi(uint16_t opcode)
{
    Instruction* i = instruction_make("SUBI", sub);
    i->size = operand_size(FRAGMENT(opcode, 7, 6));
    i->src = operand_make_immediate_value(i->size, i);
    i->dst = operand_make(FRAGMENT(opcode, 5, 0), i);

    i->base_cycles = i->size == Long ?
        cycles_immediate_instruction(i, 16, 0, 20) :
        cycles_immediate_instruction(i, 8, 0, 12);

    return i;
}

uint8_t subq(Instruction* i, M68k* ctx)
{
    // Extract the quick value, 0 represents 8
    uint32_t quick = GET(i->src, ctx);
    if (quick == 0)
        quick = 8;

    uint32_t initial = FETCH_EA_AND_GET(i->dst, ctx);
    SET(i->dst, ctx, initial - quick);

    CARRY_SET(ctx, CHECK_CARRY_SUB(initial, quick, i->size));
    OVERFLOW_SET(ctx, CHECK_OVERFLOW_SUB(initial, quick, i->size));
    ZERO_SET(ctx, initial == quick);
    NEGATIVE_SET(ctx, BIT(initial - quick, i->size - 1));
    EXTENDED_SET(ctx, CARRY(ctx));

    return 0;
}

Instruction* gen_subq(uint16_t opcode)
{
    Instruction* i = instruction_make("SUBQ", subq);
    i->size = operand_size(FRAGMENT(opcode, 7, 6));
    i->src = operand_make_value(FRAGMENT(opcode, 11, 9), i);
    i->dst = operand_make(FRAGMENT(opcode, 5, 0), i);

    i->base_cycles = i->size == Long ?
        cycles_immediate_instruction(i, 8, 8, 12) :
        cycles_immediate_instruction(i, 4, 8, 8);

    return i;
}

Instruction* gen_subx(uint16_t opcode)
{
    Instruction* i = instruction_make("SUBX", not_implemented);
    return i;
}

uint8_t tas(Instruction* i, M68k* ctx)
{
    uint32_t value = FETCH_EA_AND_GET(i->src, ctx);

    CARRY_SET(ctx, false);
    OVERFLOW_SET(ctx, false);
    ZERO_SET(ctx, value == 0);
    NEGATIVE_SET(ctx, BIT(value, 7));

    return 0;
}

Instruction* gen_tas(uint16_t opcode)
{
    Instruction* i = instruction_make("TAS", tas);
    i->size = Byte;
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);
    i->base_cycles = cycles_single_operand_instruction(i, 4, 14);
    return i;
}
