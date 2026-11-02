#include <stdio.h>
#include <stdlib.h>

#include "../utils.h"
#include "cycles.h"
#include "instruction.h"
#include "m68k.h"
#include "operands.h"

uint8_t add(Instruction* i, M68k* ctx)
{
    uint32_t b = FETCH_EA_AND_GET(&i->src, ctx);
    uint32_t a = FETCH_EA_AND_GET(&i->dst, ctx);
    SET(&i->dst, ctx, a + b);

    uint32_t result = GET(&i->dst, ctx);
    CARRY_SET(ctx, CHECK_CARRY_ADD(a, b, i->size));
    OVERFLOW_SET(ctx, CHECK_OVERFLOW_ADD(a, b, i->size));
    ZERO_SET(ctx, result == 0);
    NEGATIVE_SET(ctx, BIT(result, i->size - 1));
    EXTENDED_SET(ctx, CARRY(ctx));

    return 0;
}

void gen_add(Instruction* i, uint16_t opcode)
{
    instruction_init(i, "ADD", add);

    i->size = operand_size(FRAGMENT(opcode, 7, 6));

    int direction = BIT(opcode, 8);
    if (direction == 0)
    {
        operand_make(&i->src, FRAGMENT(opcode, 5, 0), i);
        operand_make_data_register(&i->dst, FRAGMENT(opcode, 11, 9), i);
    }
    else
    {
        operand_make_data_register(&i->src, FRAGMENT(opcode, 11, 9), i);
        operand_make(&i->dst, FRAGMENT(opcode, 5, 0), i);
    }

    i->base_cycles = i->size == Long ? cycles_standard_instruction(i, 8, 4, 8) : cycles_standard_instruction(i, 6, 6, 12); // TODO increased to eight?
}

uint8_t adda(Instruction* i, M68k* ctx)
{
    uint32_t a = FETCH_EA_AND_GET(&i->src, ctx);

    if (i->size == Word)
    {
        a = SIGN_EXTEND_W(a);
    }

    SET(&i->dst, ctx, GET(&i->dst, ctx) + a);

    return 0;
}

void gen_adda(Instruction* i, uint16_t opcode)
{
    instruction_init(i, "ADDA", adda);
    i->size = BIT(opcode, 8) ? Long : Word;
    operand_make(&i->src, FRAGMENT(opcode, 5, 0), i);
    operand_make_address_register(&i->dst, FRAGMENT(opcode, 11, 9), i);

    i->base_cycles = i->size == Long ? cycles_standard_instruction(i, 8, 4, 8) : cycles_standard_instruction(i, 6, 6, 12); // TODO increased to eight?
}

void gen_addi(Instruction* i, uint16_t opcode)
{
    instruction_init(i, "ADDI", add);
    i->size = operand_size(FRAGMENT(opcode, 7, 6));
    operand_make_immediate_value(&i->src, i->size, i);
    operand_make(&i->dst, FRAGMENT(opcode, 5, 0), i);

    i->base_cycles = i->size == Long ? cycles_immediate_instruction(i, 16, 0, 20) : cycles_immediate_instruction(i, 8, 0, 12);
}

uint8_t addq(Instruction* i, M68k* ctx)
{
    // Extract the quick value, 0 represents 8

    uint32_t quick = GET(&i->src, ctx);

    if (quick == 0)
    {
        quick = 8;
    }

    uint32_t initial = FETCH_EA_AND_GET(&i->dst, ctx);
    SET(&i->dst, ctx, initial + quick);

    // The condition codes are not altered when subtracting from address registers

    if (i->dst.type != AddressRegister)
    {
        uint32_t result = GET(&i->dst, ctx);
        CARRY_SET(ctx, CHECK_CARRY_ADD(initial, quick, i->size));
        OVERFLOW_SET(ctx, CHECK_OVERFLOW_ADD(initial, quick, i->size));
        ZERO_SET(ctx, result == 0);
        NEGATIVE_SET(ctx, BIT(result, i->size - 1));
    }

    return 0;
}

void gen_addq(Instruction* i, uint16_t opcode)
{
    instruction_init(i, "ADDQ", addq);
    i->size = operand_size(FRAGMENT(opcode, 7, 6));
    operand_make_value(&i->src, FRAGMENT(opcode, 11, 9), i);
    operand_make(&i->dst, FRAGMENT(opcode, 5, 0), i);

    i->base_cycles = i->size == Long ? cycles_immediate_instruction(i, 8, 8, 12) : cycles_immediate_instruction(i, 4, 4, 8);

    // For address registers, the entire register is used regardless of the instruction's size

    if (i->dst.type == AddressRegister)
    {
        i->size = Long;
    }
}

uint8_t addx(Instruction* i, M68k* ctx)
{
    uint32_t b = FETCH_EA_AND_GET(&i->src, ctx);
    uint32_t a = FETCH_EA_AND_GET(&i->dst, ctx);
    bool extended = EXTENDED(ctx);
    SET(&i->dst, ctx, a + b + extended);

    uint32_t result = GET(&i->dst, ctx);
    CARRY_SET(ctx, CHECK_CARRY_ADD(a, b + extended, i->size)); // TODO does this work?
    OVERFLOW_SET(ctx, CHECK_OVERFLOW_ADD(a, b + extended, i->size));
    ZERO_SET(ctx, result == 0);
    NEGATIVE_SET(ctx, BIT(result, i->size - 1));
    EXTENDED_SET(ctx, CARRY(ctx));

    return 0;
}

void gen_addx(Instruction* i, uint16_t opcode)
{
    instruction_init(i, "ADDX", addx);
    i->size = operand_size(FRAGMENT(opcode, 7, 6));

    bool data_register = BIT(opcode, 3);
    if (data_register)
    {
        operand_make_address_register_indirect_predec(&i->src, FRAGMENT(opcode, 2, 0), i);
        operand_make_address_register_indirect_predec(&i->dst, FRAGMENT(opcode, 11, 9), i);
    }
    else
    {
        operand_make_data_register(&i->src, FRAGMENT(opcode, 2, 0), i);
        operand_make_data_register(&i->dst, FRAGMENT(opcode, 11, 9), i);
    }
}

uint8_t clr(Instruction* i, M68k* ctx)
{
    FETCH_EA_AND_SET(&i->src, ctx, 0);

    CARRY_SET(ctx, false);
    OVERFLOW_SET(ctx, false);
    ZERO_SET(ctx, true);
    NEGATIVE_SET(ctx, false);

    return 0;
}

void gen_clr(Instruction* i, uint16_t opcode)
{
    instruction_init(i, "CLR", clr);
    i->size = operand_size(FRAGMENT(opcode, 7, 6));
    operand_make(&i->src, FRAGMENT(opcode, 5, 0), i);

    i->base_cycles = i->size == Long ? cycles_single_operand_instruction(i, 6, 12) : cycles_single_operand_instruction(i, 4, 8);
}

uint8_t cmp(Instruction* i, M68k* ctx)
{
    uint32_t b = FETCH_EA_AND_GET(&i->src, ctx);
    uint32_t a = FETCH_EA_AND_GET(&i->dst, ctx);

    CARRY_SET(ctx, CHECK_CARRY_SUB(a, b, i->size));
    OVERFLOW_SET(ctx, CHECK_OVERFLOW_SUB(a, b, i->size));
    ZERO_SET(ctx, a == b);
    NEGATIVE_SET(ctx, BIT(a - b, i->size - 1));

    return 0;
}

void gen_cmp(Instruction* i, uint16_t opcode)
{
    instruction_init(i, "CMP", cmp);
    i->size = operand_size(FRAGMENT(opcode, 7, 6));
    operand_make(&i->src, FRAGMENT(opcode, 5, 0), i);
    operand_make_data_register(&i->dst, FRAGMENT(opcode, 11, 9), i);

    i->base_cycles = i->size == Long ? cycles_standard_instruction(i, 6, 6, 0) : cycles_standard_instruction(i, 6, 4, 0);
}

uint8_t cmpa(Instruction* i, M68k* ctx)
{
    uint32_t b = FETCH_EA_AND_GET(&i->src, ctx);
    if (i->size == Word)
    {
        b = SIGN_EXTEND_W(b);
    }

    uint32_t a = ctx->address_registers[i->dst.n];

    CARRY_SET(ctx, CHECK_CARRY_SUB(a, b, i->size));
    OVERFLOW_SET(ctx, CHECK_OVERFLOW_SUB(a, b, i->size));
    ZERO_SET(ctx, a == b);
    NEGATIVE_SET(ctx, BIT(a - b, i->size - 1));

    return 0;
}

void gen_cmpa(Instruction* i, uint16_t opcode)
{
    instruction_init(i, "CMPA", cmpa);
    i->size = BIT(opcode, 8) ? Long : Word;
    operand_make(&i->src, FRAGMENT(opcode, 5, 0), i);
    operand_make_address_register(&i->dst, FRAGMENT(opcode, 11, 9), i);

    i->base_cycles = i->size == Long ? cycles_standard_instruction(i, 6, 6, 0) : cycles_standard_instruction(i, 6, 4, 0);
}

void gen_cmpi(Instruction* i, uint16_t opcode)
{
    instruction_init(i, "CMPI", cmp);
    i->size = operand_size(FRAGMENT(opcode, 7, 6));
    operand_make_immediate_value(&i->src, i->size, i);
    operand_make(&i->dst, FRAGMENT(opcode, 5, 0), i);

    i->base_cycles = i->size == Long ? cycles_immediate_instruction(i, 14, 0, 12) : cycles_immediate_instruction(i, 8, 0, 8);
}

void gen_cmpm(Instruction* i, uint16_t opcode)
{
    instruction_init(i, "CMPM", cmp);
    i->size = operand_size(FRAGMENT(opcode, 7, 6));
    operand_make_address_register_indirect_postinc(&i->src, FRAGMENT(opcode, 2, 0), i);
    operand_make_address_register_indirect_postinc(&i->dst, FRAGMENT(opcode, 11, 9), i);
}

uint8_t divu(Instruction* i, M68k* ctx)
{
    uint16_t a = FETCH_EA_AND_GET(&i->src, ctx);
    uint32_t b = ctx->data_registers[i->dst.n];

    if (a == 0)
    {
        printf("WARNING division by zero should be trapped!");
        // TODO: trap the divide by zero
        // operands are unaffected, flags are undefined
    }
    else
    {
        uint32_t quotient = b / a;
        uint16_t remainder = b % a;
        ctx->data_registers[i->dst.n] = remainder << 16 | (quotient & 0xFFFF);

        CARRY_SET(ctx, false);
        OVERFLOW_SET(ctx, (quotient & 0xFFFF0000) > 0);
        ZERO_SET(ctx, quotient == 0);
        NEGATIVE_SET(ctx, BIT(quotient, 15));
    }

    return 0;
}

uint8_t divs(Instruction* i, M68k* ctx)
{
    // Same as DIVU, but assuming signed operands

    int16_t a = FETCH_EA_AND_GET(&i->src, ctx);
    int32_t b = ctx->data_registers[i->dst.n];

    if (a == 0)
    {
        printf("WARNING division by zero should be trapped!");
        // TODO: trap the divide by zero
        // operands are unaffected, flags are undefined
    }
    else
    {
        int32_t quotient = b / a;
        uint32_t remainder = b % a;
        ctx->data_registers[i->dst.n] = remainder << 16 | (quotient & 0xFFFF);

        CARRY_SET(ctx, false);
        OVERFLOW_SET(ctx, (quotient & 0xFFFF0000) > 0);
        ZERO_SET(ctx, quotient == 0);
        NEGATIVE_SET(ctx, BIT(quotient, 15));
    }

    return 0;
}

void gen_divs(Instruction* i, uint16_t opcode)
{
    instruction_init(i, "DIVS", divs);
    i->size = Word;
    operand_make(&i->src, FRAGMENT(opcode, 5, 0), i);
    operand_make_data_register(&i->dst, FRAGMENT(opcode, 11, 9), i);
    i->base_cycles = cycles_standard_instruction(i, 0, 158, 0);
}

void gen_divu(Instruction* i, uint16_t opcode)
{
    instruction_init(i, "DIVU", divu);
    i->size = Word;
    operand_make(&i->src, FRAGMENT(opcode, 5, 0), i);
    operand_make_data_register(&i->dst, FRAGMENT(opcode, 11, 9), i);
    i->base_cycles = cycles_standard_instruction(i, 0, 140, 0);
}

uint8_t ext(Instruction* i, M68k* ctx)
{
    int x = GET(&i->src, ctx);

    uint32_t extended;
    switch (i->size)
    {
    case Word:
        extended = SIGN_EXTEND_B(x);
        break;
    case Long:
        extended = SIGN_EXTEND_W(x);
        break;
    default:
        FATAL("Invalid size %x", i->size);
    }

    SET(&i->src, ctx, extended);

    CARRY_SET(ctx, false);
    OVERFLOW_SET(ctx, false);
    ZERO_SET(ctx, extended == 0);
    NEGATIVE_SET(ctx, BIT(extended, i->size - 1) == 1);

    return 0;
}

void gen_ext(Instruction* i, uint16_t opcode)
{
    instruction_init(i, "EXT", ext);
    i->size = operand_sign_extension(FRAGMENT(opcode, 8, 6));
    operand_make_data_register(&i->src, FRAGMENT(opcode, 3, 0), i);
    i->base_cycles = 4;
}

uint8_t mulu(Instruction* i, M68k* ctx)
{
    uint16_t a = FETCH_EA_AND_GET(&i->src, ctx);
    uint16_t b = ctx->data_registers[i->dst.n];
    uint32_t result = (uint32_t)a * (uint32_t)b;

    ctx->data_registers[i->dst.n] = result;

    CARRY_SET(ctx, false);
    OVERFLOW_SET(ctx, false);
    ZERO_SET(ctx, result == 0);
    NEGATIVE_SET(ctx, BIT(result, 31));

    return 0;
}

void gen_mulu(Instruction* i, uint16_t opcode)
{
    instruction_init(i, "MULU", mulu);
    i->size = Word;
    operand_make(&i->src, FRAGMENT(opcode, 5, 0), i);
    operand_make_data_register(&i->dst, FRAGMENT(opcode, 11, 9), i);
    i->base_cycles = cycles_standard_instruction(i, 0, 70, 0);
}

uint8_t muls(Instruction* i, M68k* ctx)
{
    int16_t a = FETCH_EA_AND_GET(&i->src, ctx);
    int16_t b = ctx->data_registers[i->dst.n];
    uint32_t result = (uint32_t)((int32_t)a * (int32_t)b);

    ctx->data_registers[i->dst.n] = result;

    CARRY_SET(ctx, false);
    OVERFLOW_SET(ctx, false);
    ZERO_SET(ctx, result == 0);
    NEGATIVE_SET(ctx, BIT(result, 31));

    return 0;
}

void gen_muls(Instruction* i, uint16_t opcode)
{
    instruction_init(i, "MULS", muls);
    i->size = Word;
    operand_make(&i->src, FRAGMENT(opcode, 5, 0), i);
    operand_make_data_register(&i->dst, FRAGMENT(opcode, 11, 9), i);
    i->base_cycles = cycles_standard_instruction(i, 0, 70, 0);
}

uint8_t neg(Instruction* i, M68k* ctx)
{
    uint32_t value = FETCH_EA_AND_GET(&i->src, ctx);
    SET(&i->src, ctx, 0 - value);

    uint32_t result = GET(&i->src, ctx);
    CARRY_SET(ctx, CHECK_CARRY_SUB(0, value, i->size));
    OVERFLOW_SET(ctx, CHECK_OVERFLOW_SUB(0, value, i->size));
    ZERO_SET(ctx, result == 0);
    NEGATIVE_SET(ctx, BIT(result, i->size - 1));
    EXTENDED_SET(ctx, CARRY(ctx));

    return 0;
}

void gen_neg(Instruction* i, uint16_t opcode)
{
    instruction_init(i, "NEG", neg);
    i->size = operand_size(FRAGMENT(opcode, 7, 6));
    operand_make(&i->src, FRAGMENT(opcode, 5, 0), i);

    i->base_cycles = i->size == Long ? cycles_single_operand_instruction(i, 6, 12) : cycles_single_operand_instruction(i, 4, 8);
}

uint8_t negx(Instruction* i, M68k* ctx)
{
    uint32_t value = FETCH_EA_AND_GET(&i->src, ctx) + EXTENDED(ctx);
    SET(&i->src, ctx, 0 - value);

    uint32_t result = GET(&i->src, ctx);
    CARRY_SET(ctx, CHECK_CARRY_SUB(0, value, i->size));
    OVERFLOW_SET(ctx, CHECK_OVERFLOW_SUB(0, value, i->size));
    ZERO_SET(ctx, result == 0);
    NEGATIVE_SET(ctx, BIT(result, i->size - 1));
    EXTENDED_SET(ctx, CARRY(ctx));

    return 0;
}

void gen_negx(Instruction* i, uint16_t opcode)
{
    instruction_init(i, "NEGX", negx);
    i->size = operand_size(FRAGMENT(opcode, 7, 6));
    operand_make(&i->src, FRAGMENT(opcode, 5, 0), i);

    i->base_cycles = i->size == Long ? cycles_single_operand_instruction(i, 6, 12) : cycles_single_operand_instruction(i, 4, 8);
}

uint8_t sub(Instruction* i, M68k* ctx)
{
    uint32_t b = FETCH_EA_AND_GET(&i->src, ctx);
    uint32_t a = FETCH_EA_AND_GET(&i->dst, ctx);
    SET(&i->dst, ctx, a - b);

    CARRY_SET(ctx, CHECK_CARRY_SUB(a, b, i->size));
    OVERFLOW_SET(ctx, CHECK_OVERFLOW_SUB(a, b, i->size));
    ZERO_SET(ctx, a == b);
    NEGATIVE_SET(ctx, BIT(a - b, i->size - 1));
    EXTENDED_SET(ctx, CARRY(ctx));

    return 0;
}

void gen_sub(Instruction* i, uint16_t opcode)
{
    instruction_init(i, "SUB", sub);
    i->size = operand_size(FRAGMENT(opcode, 7, 6));

    int direction = BIT(opcode, 8);
    if (direction == 0)
    {
        operand_make(&i->src, FRAGMENT(opcode, 5, 0), i);
        operand_make_data_register(&i->dst, FRAGMENT(opcode, 11, 9), i);
    }
    else
    {
        operand_make_data_register(&i->src, FRAGMENT(opcode, 11, 9), i);
        operand_make(&i->dst, FRAGMENT(opcode, 5, 0), i);
    }

    i->base_cycles = i->size == Long ? cycles_standard_instruction(i, 8, 4, 8) : cycles_standard_instruction(i, 6, 6, 12); // TODO increase to eight?!
}

uint8_t suba(Instruction* i, M68k* ctx)
{
    uint32_t a = FETCH_EA_AND_GET(&i->src, ctx);

    if (i->size == Word)
    {
        a = SIGN_EXTEND_W(a);
    }

    SET(&i->dst, ctx, GET(&i->dst, ctx) - a);

    return 0;
}

void gen_suba(Instruction* i, uint16_t opcode)
{
    instruction_init(i, "SUBA", suba);
    i->size = BIT(opcode, 8) ? Long : Word;
    operand_make(&i->src, FRAGMENT(opcode, 5, 0), i);
    operand_make_address_register(&i->dst, FRAGMENT(opcode, 11, 9), i);
}

void gen_subi(Instruction* i, uint16_t opcode)
{
    instruction_init(i, "SUBI", sub);
    i->size = operand_size(FRAGMENT(opcode, 7, 6));
    operand_make_immediate_value(&i->src, i->size, i);
    operand_make(&i->dst, FRAGMENT(opcode, 5, 0), i);

    i->base_cycles = i->size == Long ? cycles_immediate_instruction(i, 16, 0, 20) : cycles_immediate_instruction(i, 8, 0, 12);
}

uint8_t subq(Instruction* i, M68k* ctx)
{
    // Extract the quick value, 0 represents 8

    uint32_t quick = GET(&i->src, ctx);
    if (quick == 0)
    {
        quick = 8;
    }

    uint32_t initial = FETCH_EA_AND_GET(&i->dst, ctx);
    SET(&i->dst, ctx, initial - quick);

    // The condition codes are not altered when subtracting from address registers

    if (i->dst.type != AddressRegister)
    {
        CARRY_SET(ctx, CHECK_CARRY_SUB(initial, quick, i->size));
        OVERFLOW_SET(ctx, CHECK_OVERFLOW_SUB(initial, quick, i->size));
        ZERO_SET(ctx, initial == quick);
        NEGATIVE_SET(ctx, BIT(initial - quick, i->size - 1));
        EXTENDED_SET(ctx, CARRY(ctx));
    }

    return 0;
}

void gen_subq(Instruction* i, uint16_t opcode)
{
    instruction_init(i, "SUBQ", subq);
    i->size = operand_size(FRAGMENT(opcode, 7, 6));
    operand_make_value(&i->src, FRAGMENT(opcode, 11, 9), i);
    operand_make(&i->dst, FRAGMENT(opcode, 5, 0), i);

    i->base_cycles = i->size == Long ? cycles_immediate_instruction(i, 8, 8, 12) : cycles_immediate_instruction(i, 4, 8, 8);

    // For address registers, the entire register is used regardless of the instruction's size

    if (i->dst.type == AddressRegister)
    {
        i->size = Long;
    }
}

uint8_t subx(Instruction* i, M68k* ctx)
{
    uint32_t b = FETCH_EA_AND_GET(&i->src, ctx);
    uint32_t a = FETCH_EA_AND_GET(&i->dst, ctx);
    bool extended = EXTENDED(ctx);
    SET(&i->dst, ctx, a - b - extended);

    uint32_t result = GET(&i->dst, ctx);
    CARRY_SET(ctx, CHECK_CARRY_SUB(a, b + extended, i->size)); // TODO does this work?
    OVERFLOW_SET(ctx, CHECK_OVERFLOW_SUB(a, b + extended, i->size));
    ZERO_SET(ctx, result == 0);
    NEGATIVE_SET(ctx, BIT(result, i->size - 1));
    EXTENDED_SET(ctx, CARRY(ctx));

    return 0;
}

void gen_subx(Instruction* i, uint16_t opcode)
{
    instruction_init(i, "SUBX", subx);
    i->size = operand_size(FRAGMENT(opcode, 7, 6));

    bool data_register = BIT(opcode, 3);
    if (data_register)
    {
        operand_make_address_register_indirect_predec(&i->src, FRAGMENT(opcode, 2, 0), i);
        operand_make_address_register_indirect_predec(&i->dst, FRAGMENT(opcode, 11, 9), i);
    }
    else
    {
        operand_make_data_register(&i->src, FRAGMENT(opcode, 2, 0), i);
        operand_make_data_register(&i->dst, FRAGMENT(opcode, 11, 9), i);
    }
}

uint8_t tas(Instruction* i, M68k* ctx)
{
    uint32_t value = FETCH_EA_AND_GET(&i->src, ctx);

    // TAS should set the high-order bit but this only works
    // for direct register access due to a bug in the Genesis
    // https://board.byuu.org/viewtopic.php?p=44353#p44353

    if (i->src.type == DataRegister)
    {
        SET(&i->src, ctx, value | 0x80);
    }

    CARRY_SET(ctx, false);
    OVERFLOW_SET(ctx, false);
    ZERO_SET(ctx, value == 0);
    NEGATIVE_SET(ctx, BIT(value, 7));

    return 0;
}

void gen_tas(Instruction* i, uint16_t opcode)
{
    instruction_init(i, "TAS", tas);
    i->size = Byte;
    operand_make(&i->src, FRAGMENT(opcode, 5, 0), i);
    i->base_cycles = cycles_single_operand_instruction(i, 4, 14);
}
