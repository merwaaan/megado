#include <stdbool.h>
#include <stdio.h>

#include "bit_utils.h"
#include "conditions.h"
#include "cycles.h"
#include "instruction.h"
#include "m68k.h"
#include "operands.h"

void gen_boolean_instruction(Instruction* i, uint16_t opcode, char* name, InstructionFunc* func)
{
    instruction_init(i, name, func);
    i->size = operand_size(FRAGMENT(opcode, 7, 6));

    int direction = BIT(opcode, 8);
    if (direction == 1)
    {
        operand_make_data_register(&i->src, FRAGMENT(opcode, 11, 9), i);
        operand_make(&i->dst, FRAGMENT(opcode, 5, 0), i);
    }
    else
    {
        operand_make(&i->src, FRAGMENT(opcode, 5, 0), i);
        operand_make_data_register(&i->dst, FRAGMENT(opcode, 11, 9), i);
    }
}

void gen_boolean_instruction_immediate(Instruction* i, uint16_t opcode, char* name, InstructionFunc* func)
{
    instruction_init(i, name, func);
    i->size = operand_size(FRAGMENT(opcode, 7, 6));
    operand_make_immediate_value(&i->src, i->size, i);
    operand_make(&i->dst, FRAGMENT(opcode, 5, 0), i);
}

uint8_t and(Instruction* i, M68k* ctx)
{
    // Fetch both effective addresses to cover the two variants: AND ea, Dn & AND Dn, ea

    uint32_t result = FETCH_EA_AND_GET(&i->src, ctx) & FETCH_EA_AND_GET(&i->dst, ctx);
    SET(&i->dst, ctx, result);

    CARRY_SET(ctx, false);
    OVERFLOW_SET(ctx, false);
    ZERO_SET(ctx, result == 0);
    NEGATIVE_SET(ctx, BIT(result, i->size - 1));

    return 0;
}

void gen_and(Instruction* i, uint16_t opcode)
{
    gen_boolean_instruction(i, opcode, "AND", and);

    i->base_cycles = i->size == Long ? cycles_standard_instruction(i, 0, 6, 12) : cycles_standard_instruction(i, 0, 4, 8); // TODO increased to eight?!
}

void gen_andi(Instruction* i, uint16_t opcode)
{
    gen_boolean_instruction_immediate(i, opcode, "ANDI", and);

    i->base_cycles = i->size == Long ? cycles_immediate_instruction(i, 14, 0, 20) : cycles_immediate_instruction(i, 8, 0, 12);
}

uint8_t andi_ccr(Instruction* i, M68k* ctx)
{
    ctx->status = (ctx->status & 0xFFE0) | (ctx->status & FETCH_EA_AND_GET(&i->src, ctx) & 0x1F);

    return 0;
}

void gen_andi_ccr(Instruction* i, uint16_t opcode)
{
    instruction_init(i, "ANDI CCR", andi_ccr);
    operand_make_immediate_value(&i->src, Byte, i);
    i->base_cycles = 20;
}

uint8_t eor(Instruction* i, M68k* ctx)
{
    uint32_t result = FETCH_EA_AND_GET(&i->src, ctx) ^ FETCH_EA_AND_GET(&i->dst, ctx);
    SET(&i->dst, ctx, result);

    CARRY_SET(ctx, false);
    OVERFLOW_SET(ctx, false);
    ZERO_SET(ctx, result == 0);
    NEGATIVE_SET(ctx, BIT(result, i->size - 1) == 1);

    return 0;
}

void gen_eor(Instruction* i, uint16_t opcode)
{
    gen_boolean_instruction(i, opcode, "EOR", eor);

    i->base_cycles = i->size == Long ? cycles_standard_instruction(i, 0, 8, 12) : cycles_standard_instruction(i, 0, 4, 6);
}

void gen_eori(Instruction* i, uint16_t opcode)
{
    gen_boolean_instruction_immediate(i, opcode, "EORI", eor);

    i->base_cycles = i->size == Long ? cycles_immediate_instruction(i, 16, 0, 20) : cycles_immediate_instruction(i, 8, 0, 12);
}

uint8_t eori_ccr(Instruction* i, M68k* ctx)
{
    ctx->status = (ctx->status & 0xFFE0) | ((ctx->status ^ FETCH_EA_AND_GET(&i->src, ctx)) & 0x1F);

    return 0;
}

void gen_eori_ccr(Instruction* i, uint16_t opcode)
{
    instruction_init(i, "EORI CCR", eori_ccr);
    operand_make_immediate_value(&i->src, Byte, i);
    i->base_cycles = 20;
}

uint8_t or (Instruction * i, M68k* ctx)
{
    uint32_t result = FETCH_EA_AND_GET(&i->src, ctx) | FETCH_EA_AND_GET(&i->dst, ctx);
    SET(&i->dst, ctx, result);

    CARRY_SET(ctx, false);
    OVERFLOW_SET(ctx, false);
    ZERO_SET(ctx, result == 0);
    NEGATIVE_SET(ctx, BIT(result, i->size - 1) == 1);

    return 0;
}

void gen_or(Instruction* i, uint16_t opcode)
{
    gen_boolean_instruction(i, opcode, "OR", or);

    i->base_cycles = i->size == Long ? cycles_standard_instruction(i, 0, 6, 12) : cycles_standard_instruction(i, 0, 4, 8); // TODO increase to eight?!
}

void gen_ori(Instruction* i, uint16_t opcode)
{
    gen_boolean_instruction_immediate(i, opcode, "ORI", or);

    i->base_cycles = i->size == Long ? cycles_immediate_instruction(i, 16, 0, 20) : cycles_immediate_instruction(i, 8, 0, 12);
}

uint8_t ori_ccr(Instruction* i, M68k* ctx)
{
    ctx->status = (ctx->status & 0xFFE0) | ((ctx->status | FETCH_EA_AND_GET(&i->src, ctx)) & 0x1F);

    return 0;
}

void gen_ori_ccr(Instruction* i, uint16_t opcode)
{
    instruction_init(i, "ORI CCR", ori_ccr);
    operand_make_immediate_value(&i->src, Byte, i);
    i->base_cycles = 20;
}

uint8_t not(Instruction * i, M68k* ctx)
{
    FETCH_EA(&i->dst, ctx);
    SET(&i->dst, ctx, ~GET(&i->dst, ctx));

    uint32_t result = GET(&i->dst, ctx);
    CARRY_SET(ctx, false);
    OVERFLOW_SET(ctx, false);
    ZERO_SET(ctx, result == 0);
    NEGATIVE_SET(ctx, BIT(result, i->size - 1) == 1);

    return 0;
}

void gen_not(Instruction* i, uint16_t opcode)
{
    instruction_init(i, "NOT", not);
    i->size = operand_size(FRAGMENT(opcode, 7, 6));
    operand_make(&i->dst, FRAGMENT(opcode, 5, 0), i);

    i->base_cycles = i->size == Long ? cycles_single_operand_instruction(i, 6, 12) : cycles_single_operand_instruction(i, 4, 8);
}

uint8_t scc(Instruction* i, M68k* ctx)
{
    FETCH_EA_AND_SET(&i->dst, ctx, i->condition->func(ctx) ? 0xFF : 0);

    return 0;
}

void gen_scc(Instruction* i, uint16_t opcode)
{
    Condition* condition = condition_get(FRAGMENT(opcode, 11, 8));

    // Format the instruction name depending on its internal condition

    char name[5];
    sprintf(name, "S%s", condition->mnemonics);

    instruction_init(i, name, scc);
    i->size = Byte;
    i->condition = condition;
    operand_make(&i->dst, FRAGMENT(opcode, 5, 0), i);

    i->base_cycles = i->size == Long ? cycles_single_operand_instruction(i, 6, 8) : cycles_single_operand_instruction(i, 4, 8);
}

uint8_t tst(Instruction* i, M68k* ctx)
{
    uint32_t value = FETCH_EA_AND_GET(&i->dst, ctx);

    CARRY_SET(ctx, false);
    OVERFLOW_SET(ctx, false);
    ZERO_SET(ctx, value == 0);
    NEGATIVE_SET(ctx, BIT(value, i->size - 1) == 1);

    return 0;
}

void gen_tst(Instruction* i, uint16_t opcode)
{
    instruction_init(i, "TST", tst);
    i->size = operand_size(FRAGMENT(opcode, 7, 6));
    operand_make(&i->dst, FRAGMENT(opcode, 5, 0), i);
    i->base_cycles = cycles_single_operand_instruction(i, 4, 4);
}
