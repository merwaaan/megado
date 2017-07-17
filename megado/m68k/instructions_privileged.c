#include <stdlib.h>

#include "cycles.h"
#include "instruction.h"
#include "m68k.h"
#include "operands.h"

uint8_t andi_sr(Instruction* i, M68k* ctx)
{
    ctx->status &= FETCH_EA_AND_GET(i->src, ctx);

    return 0;
}

Instruction* gen_andi_sr(uint16_t opcode)
{
    Instruction* i = instruction_make("ANDI SR", andi_sr);
    i->src = operand_make_immediate_value(Word, i);
    i->base_cycles = 20;
    return i;
}

uint8_t eori_sr(Instruction* i, M68k* ctx)
{
    ctx->status ^= FETCH_EA_AND_GET(i->src, ctx);

    return 0;
}

Instruction* gen_eori_sr(uint16_t opcode)
{
    Instruction* i = instruction_make("EORI SR", eori_sr);
    i->src = operand_make_immediate_value(Word, i);
    i->base_cycles = 20;
    return i;
}

uint8_t ori_sr(Instruction* i, M68k* ctx)
{
    ctx->status |= FETCH_EA_AND_GET(i->src, ctx);

    return 0;
}

Instruction* gen_ori_sr(uint16_t opcode)
{
    Instruction* i = instruction_make("ORI SR", ori_sr);
    i->src = operand_make_immediate_value(Word, i);
    i->base_cycles = 20;
    return i;
}

uint8_t move_from_sr(Instruction* i, M68k* ctx)
{
    FETCH_EA_AND_SET(i->dst, ctx, ctx->status);

    return 0;
}

Instruction* gen_move_from_sr(uint16_t opcode)
{
    Instruction* i = instruction_make("MOVE from SR", move_from_sr);
    i->size = Word;
    i->dst = operand_make(FRAGMENT(opcode, 5, 0), i);
    i->base_cycles = 6;
    return i;
}

uint8_t move_to_sr(Instruction* i, M68k* ctx)
{
    ctx->status = FETCH_EA_AND_GET(i->src, ctx);

    return 0;
}

Instruction* gen_move_to_sr(uint16_t opcode)
{
    Instruction* i = instruction_make("MOVE to SR", move_to_sr);
    i->size = Word;
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);
    i->base_cycles = 12;
    return i;
}

uint8_t move_usp(Instruction* i, M68k* ctx)
{
    // Register -> user stack pointer
    if (i->src != NULL)
        ctx->usp = GET(i->src, ctx);
    // User stack pointer -> register
    else
        SET(i->dst, ctx, ctx->usp);

    return 0;
}

Instruction* gen_move_usp(uint16_t opcode)
{
    Instruction* i = instruction_make("MOVE USP", move_usp);
    i->size = Long;

    Operand* reg = operand_make_address_register(FRAGMENT(opcode, 2, 0), i);

    int direction = BIT(opcode, 3);
    if (direction)
        i->dst = reg;
    else
        i->src = reg;

    i->base_cycles = 4;

    return i;
} // TODO cycles

Instruction* gen_reset(uint16_t opcode)
{
    Instruction* i = instruction_make("RESET", not_implemented);
    i->base_cycles = 132;
    return i;
}

uint8_t rte(Instruction* i, M68k* ctx)
{
    ctx->status = m68k_read_w(ctx, ctx->address_registers[7]);
    ctx->address_registers[7] += 2;
    ctx->pc = m68k_read_l(ctx, ctx->address_registers[7]) & M68K_ADDRESS_WIDTH;
    ctx->address_registers[7] += 4;

    return 0;
}

Instruction* gen_rte(uint16_t opcode)
{
    Instruction* i = instruction_make("RTE", rte);
    i->base_cycles = 20;
    return i;
}

Instruction* gen_stop(uint16_t opcode)
{
    Instruction* i = instruction_make("STOP", not_implemented);
    i->base_cycles = 4;
    return i;
}
