#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "bit_utils.h"
#include "conditions.h"
#include "instruction.h"
#include "m68k.h"
#include "operands.h"

int bcc(Instruction* i, M68k* ctx)
{
    int16_t offset = FETCH_EA_AND_GET(i->src, ctx);

    // If the condition is true, jump from (instruction address + 2)
    if (i->condition->func(ctx))
    {
        ctx->pc = ctx->instruction_address + 2 + (i->size == Byte ? (int8_t)offset : (int16_t)offset);
        return 10;
    }

    return i->size == Byte ? 8 : 12;
}

Instruction* gen_bcc(uint16_t opcode)
{
    Condition* condition = condition_get(FRAGMENT(opcode, 11, 8));

    // Format the instruction name depending on its internal condition
    char name[5];
    sprintf(name, "B%s", condition->mnemonics);

    Instruction* i = instruction_make(name, bcc);
    i->condition = condition;

    int displacement = FRAGMENT(opcode, 7, 0);
    if (displacement == 0)
    {
        i->size = Word;
        i->src = operand_make_branching_offset(i, Word);
    }
    else
    {
        i->size = Byte;
        i->src = operand_make_branching_offset(i, Byte);
    }

    return i;
}

int bra(Instruction* i, M68k* ctx)
{
    // Jump from (instruction address + 2)
    int16_t offset = FETCH_EA_AND_GET(i->src, ctx);
    ctx->pc = ctx->instruction_address + 2 + (i->size == Byte ? (int8_t)offset : (int16_t)offset);

    return 0;
}

Instruction* gen_bra(uint16_t opcode)
{
    Instruction* i = instruction_make("BRA", bra);
    i->base_cycles = 10;

    int offset = FRAGMENT(opcode, 7, 0);
    if (offset == 0)
    {
        i->size = Word;
        i->src = operand_make_branching_offset(i, Word);
    }
    else
    {
        i->size = Byte;
        i->src = operand_make_branching_offset(i, Byte);
    }

    return i;
}

int bsr(Instruction* i, M68k* ctx)
{
    int16_t offset = FETCH_EA_AND_GET(i->src, ctx);

    // Push the address that follows the instruction on the stack
    ctx->address_registers[7] -= 4;
    m68k_write_l(ctx, ctx->address_registers[7], ctx->pc);

    // Jump from (instruction address + 2)
    ctx->pc = ctx->instruction_address + 2 + (i->size == Byte ? (int8_t)offset : (int16_t)offset);

    return 0;
}

Instruction* gen_bsr(uint16_t opcode)
{
    Instruction* i = instruction_make("BSR", bsr);
    i->base_cycles = 18;

    uint8_t offset = FRAGMENT(opcode, 7, 0);
    i->size = offset == 0 ? Word : Byte;
    i->src = operand_make_branching_offset(i, i->size);
    
    return i;
}

int dbcc(Instruction* i, M68k* ctx)
{
    int16_t offset = FETCH_EA_AND_GET(i->dst, ctx);

    // If the condition is true, no operation is performed
    if (i->condition->func(ctx))
        return 12;

    bool branch_taken = false;

    // If the counter is still positive, jump from (instruction address + 2) 
    uint16_t reg = GET(i->src, ctx);
    if (reg > 0)
    {
        ctx->pc = ctx->instruction_address + 2 + offset;
        branch_taken = true;
    }

    // Decrement the counter
    SET(i->src, ctx, reg - 1);

    return branch_taken ? 10 : 14;
}

Instruction* gen_dbcc(uint16_t opcode)
{
    Condition* condition = condition_get(FRAGMENT(opcode, 11, 8));

    // Format the instruction name depending on its internal condition
    char name[5];
    sprintf(name, "DB%s", condition->mnemonics);

    Instruction* i = instruction_make(name, dbcc);
    i->size = Word;
    i->condition = condition;
    i->src = operand_make_data_register(FRAGMENT(opcode, 2, 0), i);
    i->dst = operand_make_branching_offset(i, Word);
    return i;
}

int jmp(Instruction* i, M68k* ctx)
{
    ctx->pc = FETCH_EA(i->src, ctx);

    return 0;
}

Instruction* gen_jmp(uint16_t opcode)
{
    Instruction* i = instruction_make("JMP", jmp);
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);
    return i;
}

int jsr(Instruction* i, M68k* ctx)
{
    uint32_t ea = FETCH_EA(i->src, ctx);

    if (i->src->type == AbsoluteShort)
        ea = SIGN_EXTEND_W(ea);

    // Push the address following the instruction onto the stack
    ctx->address_registers[7] -= 4;
    m68k_write_l(ctx, ctx->address_registers[7], ctx->pc);

    // Jump to the location specified by ea
    ctx->pc = ea;

    return 0;
}

Instruction* gen_jsr(uint16_t opcode)
{
    Instruction* i = instruction_make("JSR", jsr);
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);
    i->size = Long;
    return i;
}

int nop(Instruction* i, M68k* ctx)
{
    return 0;
}

Instruction* gen_nop(uint16_t opcode)
{
    Instruction* i = instruction_make("NOP", nop);
    i->base_cycles = 4;
    return i;
}

Instruction* gen_rtd(uint16_t opcode)
{
    Instruction* i = instruction_make("RTD", not_implemented);
    return i;
}

int rtr(Instruction* i, M68k* ctx)
{
    uint8_t ccr = m68k_read_w(ctx, ctx->address_registers[7]);
    ctx->status = (ctx->status & 0xFFE0) | (ccr & 0x1F);

    uint32_t pc = m68k_read_l(ctx, ctx->address_registers[7] + 2);
    ctx->pc = pc;

    ctx->address_registers[7] += 6;

    return 0;
}

Instruction* gen_rtr(uint16_t opcode)
{
    Instruction* i = instruction_make("RTR", rtr);
    i->base_cycles = 20;
    return i;
}

int rts(Instruction* i, M68k* ctx)
{
    ctx->pc = m68k_read_l(ctx, ctx->address_registers[7]);
    ctx->address_registers[7] += 4;

    return 0;
}

Instruction* gen_rts(uint16_t opcode)
{
    Instruction* i = instruction_make("RTS", rts);
    i->base_cycles = 16;
    return i;
}
