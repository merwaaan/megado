#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "bit_utils.h"
#include "conditions.h"
#include "cycles.h"
#include "instruction.h"
#include "m68k.h"
#include "operands.h"

Instruction* gen_boolean_instruction(uint16_t opcode, char* name, InstructionFunc* func)
{
    Instruction* i = instruction_make(name, func);
    i->size = operand_size(FRAGMENT(opcode, 7, 6));

    Operand* reg = operand_make_data_register(FRAGMENT(opcode, 11, 9), i);
    Operand* ea = operand_make(FRAGMENT(opcode, 5, 0), i);

    int direction = BIT(opcode, 8);
    if (direction == 1)
    {
        i->src = reg;
        i->dst = ea;
    }
    else
    {
        i->src = ea;
        i->dst = reg;
    }

    return i;
}

Instruction* gen_boolean_instruction_immediate(uint16_t opcode, char* name, InstructionFunc* func)
{
    Instruction* i = instruction_make(name, func);
    i->size = operand_size(FRAGMENT(opcode, 7, 6));
    i->src = operand_make_immediate_value(i->size, i);
    i->dst = operand_make(FRAGMENT(opcode, 5, 0), i);
    return i;
}

int and(Instruction* i, M68k* ctx)
{
    // Fetch both effective addresses to cover the two variants: AND ea, Dn & AND Dn, ea
    uint32_t result = FETCH_EA_AND_GET(i->src, ctx) & FETCH_EA_AND_GET(i->dst, ctx);
    SET(i->dst, ctx, result);

    CARRY_SET(ctx, false);
    OVERFLOW_SET(ctx, false);
    ZERO_SET(ctx, result == 0);
    NEGATIVE_SET(ctx, BIT(result, i->size - 1));

    return 0;
}

Instruction* gen_and(uint16_t opcode)
{
    Instruction* i = gen_boolean_instruction(opcode, "AND", and);

    i->base_cycles = i->size == Long ?
        cycles_standard_instruction(i, 0, 6, 12) :
        cycles_standard_instruction(i, 0, 4, 8); // TODO increased to eight?!

    return i;
}

Instruction* gen_andi(uint16_t opcode)
{
    Instruction* i = gen_boolean_instruction_immediate(opcode, "ANDI", and);

    i->base_cycles = i->size == Long ?
        cycles_immediate_instruction(i, 14, 0, 20) :
        cycles_immediate_instruction(i, 8, 0, 12);

    return i;
}

int andi_ccr(Instruction* i, M68k* ctx)
{
    ctx->status = (ctx->status & 0xFFE0) | (ctx->status & FETCH_EA_AND_GET(i->src, ctx) & 0x1F);

    return 0;
}

Instruction* gen_andi_ccr(uint16_t opcode)
{
    Instruction* i = instruction_make("ANDI CCR", andi_ccr);
    i->src = operand_make_immediate_value(Byte, i);
    i->base_cycles = 20;
    return i;
}

int eor(Instruction* i, M68k* ctx)
{
    uint32_t result = FETCH_EA_AND_GET(i->src, ctx) ^ FETCH_EA_AND_GET(i->dst, ctx);
    SET(i->dst, ctx, result);

    CARRY_SET(ctx, false);
    OVERFLOW_SET(ctx, false);
    ZERO_SET(ctx, result == 0);
    NEGATIVE_SET(ctx, result < 0);

    return 0;
}

Instruction* gen_eor(uint16_t opcode)
{
    Instruction* i = gen_boolean_instruction(opcode, "EOR", eor);

    i->base_cycles = i->size == Long ?
        cycles_standard_instruction(i, 0, 8, 12) :
        cycles_standard_instruction(i, 0, 4, 6);

    return i;
}

Instruction* gen_eori(uint16_t opcode)
{
    Instruction* i = gen_boolean_instruction_immediate(opcode, "EORI", eor);

    i->base_cycles = i->size == Long ?
        cycles_immediate_instruction(i, 16, 0, 20) :
        cycles_immediate_instruction(i, 8, 0, 12);

    return i;
}

int eori_ccr(Instruction* i, M68k* ctx)
{
    ctx->status = (ctx->status & 0xFFE0) | ((ctx->status ^ FETCH_EA_AND_GET(i->src, ctx)) & 0x1F);

    return 0;
}

Instruction* gen_eori_ccr(uint16_t opcode)
{
    Instruction* i = instruction_make("EORI CCR", eori_ccr);
    i->src = operand_make_immediate_value(Byte, i);
    i->base_cycles = 20;
    return i;
}

int or (Instruction* i, M68k* ctx)
{
    uint32_t result = FETCH_EA_AND_GET(i->src, ctx) | FETCH_EA_AND_GET(i->dst, ctx);
    SET(i->dst, ctx, result);

    CARRY_SET(ctx, false);
    OVERFLOW_SET(ctx, false);
    ZERO_SET(ctx, result == 0);
    NEGATIVE_SET(ctx, result < 0);

    return 0;
}

Instruction* gen_or(uint16_t opcode)
{
    Instruction* i = gen_boolean_instruction(opcode, "OR", or );

    i->base_cycles = i->size == Long ?
        cycles_standard_instruction(i, 0, 6, 12) :
        cycles_standard_instruction(i, 0, 4, 8); // TODO increase to eight?!

    return i;
}

Instruction* gen_ori(uint16_t opcode)
{
    Instruction* i = gen_boolean_instruction_immediate(opcode, "ORI", or );

    i->base_cycles = i->size == Long ?
        cycles_immediate_instruction(i, 16, 0, 20) :
        cycles_immediate_instruction(i, 8, 0, 12);

    return i;
}

int ori_ccr(Instruction* i, M68k* ctx)
{
    ctx->status = (ctx->status & 0xFFE0) | ((ctx->status | FETCH_EA_AND_GET(i->src, ctx)) & 0x1F);

    return 0;
}

Instruction* gen_ori_ccr(uint16_t opcode)
{
    Instruction* i = instruction_make("ORI CCR", ori_ccr);
    i->src = operand_make_immediate_value(Byte, i);
    i->base_cycles = 20;
    return i;
}

int not(Instruction* i, M68k* ctx)
{
    FETCH_EA(i->src, ctx);
    SET(i->src, ctx, ~GET(i->src, ctx));

    uint32_t result = GET(i->src, ctx);
    CARRY_SET(ctx, false);
    OVERFLOW_SET(ctx, false);
    ZERO_SET(ctx, result == 0);
    NEGATIVE_SET(ctx, BIT(result, i->size - 1) == 1);

    return 0;
}

Instruction* gen_not(uint16_t opcode)
{
    Instruction* i = instruction_make("NOT", not);
    i->size = operand_size(FRAGMENT(opcode, 7, 6));
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);

    i->base_cycles = i->size == Long ?
        cycles_single_operand_instruction(i, 6, 12) :
        cycles_single_operand_instruction(i, 4, 8);

    return i;
}

int scc(Instruction* i, M68k* ctx)
{
    FETCH_EA_AND_SET(i->src, ctx, i->condition->func(ctx) ? 0xFF : 0);

    return 0;
}

Instruction* gen_scc(uint16_t opcode)
{
    Condition* condition = condition_get(FRAGMENT(opcode, 11, 8));

    // Format the instruction name depending on its internal condition
    char name[5];
    sprintf(name, "S%s", condition->mnemonics);

    Instruction* i = instruction_make(name, scc);
    i->size = Byte;
    i->condition = condition;
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);

    i->base_cycles = i->size == Long ?
        cycles_single_operand_instruction(i, 6, 8) :
        cycles_single_operand_instruction(i, 4, 8);

    return i;
}

int tst(Instruction* i, M68k* ctx)
{
    uint32_t value = FETCH_EA_AND_GET(i->src, ctx);

    CARRY_SET(ctx, false);
    OVERFLOW_SET(ctx, false);
    ZERO_SET(ctx, value == 0);
    NEGATIVE_SET(ctx, BIT(value, i->size - 1) == 1);

    return 0;
}

Instruction* gen_tst(uint16_t opcode)
{ // TODo check subtlety with size!! http://oldwww.nvg.ntnu.no/amiga/MC680x0_Sections/tst.HTML
    Instruction* i = instruction_make("TST", tst);
    i->size = operand_size(FRAGMENT(opcode, 7, 6));
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);
    i->base_cycles = cycles_single_operand_instruction(i, 4, 4);
    return i;
}
