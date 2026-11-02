#include <stdlib.h>

#include "../genesis.h"
#include "instruction.h"
#include "m68k.h"
#include "map.h"
#include "operands.h"

uint8_t andi_sr(Instruction* i, M68k* ctx)
{
    ctx->status &= FETCH_EA_AND_GET(&i->src, ctx) & STATUS_MASK;

    return 0;
}

void gen_andi_sr(Instruction* i, uint16_t opcode)
{
    instruction_init(i, "ANDI SR", andi_sr);
    operand_make_immediate_value(&i->src, Word, i);
    i->base_cycles = 20;
}

uint8_t eori_sr(Instruction* i, M68k* ctx)
{
    ctx->status ^= FETCH_EA_AND_GET(&i->src, ctx) & STATUS_MASK;

    return 0;
}

void gen_eori_sr(Instruction* i, uint16_t opcode)
{
    instruction_init(i, "EORI SR", eori_sr);
    operand_make_immediate_value(&i->src, Word, i);
    i->base_cycles = 20;
}

uint8_t ori_sr(Instruction* i, M68k* ctx)
{
    ctx->status |= FETCH_EA_AND_GET(&i->src, ctx) & STATUS_MASK;

    return 0;
}

void gen_ori_sr(Instruction* i, uint16_t opcode)
{
    instruction_init(i, "ORI SR", ori_sr);
    operand_make_immediate_value(&i->src, Word, i);
    i->base_cycles = 20;
}

uint8_t move_from_sr(Instruction* i, M68k* ctx)
{
    FETCH_EA_AND_SET(&i->dst, ctx, ctx->status);

    return 0;
}

void gen_move_from_sr(Instruction* i, uint16_t opcode)
{
    instruction_init(i, "MOVE from SR", move_from_sr);
    i->size = Word;
    operand_make(&i->dst, FRAGMENT(opcode, 5, 0), i);
    i->base_cycles = 6;
}

uint8_t move_to_sr(Instruction* i, M68k* ctx)
{
    ctx->status = FETCH_EA_AND_GET(&i->src, ctx) & STATUS_MASK;

    return 0;
}

void gen_move_to_sr(Instruction* i, uint16_t opcode)
{
    instruction_init(i, "MOVE to SR", move_to_sr);
    i->size = Word;
    operand_make(&i->src, FRAGMENT(opcode, 5, 0), i);
    i->base_cycles = 12;
}

uint8_t move_usp(Instruction* i, M68k* ctx)
{
    // Register -> user stack pointer

    if (i->src.type != NoOperand)
    {
        ctx->usp = GET(&i->src, ctx);
    }

    // User stack pointer -> register

    else
    {
        SET(&i->dst, ctx, ctx->usp);
    }

    return 0;
}

void gen_move_usp(Instruction* i, uint16_t opcode)
{
    instruction_init(i, "MOVE USP", move_usp);
    i->size = Long;

    int direction = BIT(opcode, 3);
    if (direction)
    {
        operand_make_address_register(&i->dst, FRAGMENT(opcode, 2, 0), i);
    }
    else
    {
        operand_make_address_register(&i->src, FRAGMENT(opcode, 2, 0), i);
    }

    i->base_cycles = 4;
} // TODO cycles

void gen_reset(Instruction* i, uint16_t opcode)
{
    instruction_init(i, "RESET", not_implemented);
    i->base_cycles = 132;
}

uint8_t rte(Instruction* i, M68k* ctx)
{
    ctx->status = map_read_w(ctx->genesis->map, ctx->address_registers[7]) & STATUS_MASK;
    ctx->address_registers[7] += 2;
    ctx->pc = map_read_l(ctx->genesis->map, ctx->address_registers[7]) & M68K_ADDRESS_WIDTH;
    ctx->address_registers[7] += 4;

    return 0;
}

void gen_rte(Instruction* i, uint16_t opcode)
{
    instruction_init(i, "RTE", rte);
    i->base_cycles = 20;
}

uint8_t stop(Instruction* i, M68k* ctx)
{
    ctx->stopped = true;
    return 0;
}

void gen_stop(Instruction* i, uint16_t opcode)
{
    instruction_init(i, "STOP", stop);
    i->base_cycles = 4;
}
