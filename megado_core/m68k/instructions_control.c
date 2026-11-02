#include <stdbool.h>
#include <stdio.h>

#include "../genesis.h"
#include "bit_utils.h"
#include "conditions.h"
#include "instruction.h"
#include "m68k.h"
#include "map.h"
#include "operands.h"

uint8_t bcc(Instruction* i, M68k* ctx)
{
    int16_t offset = FETCH_EA_AND_GET(&i->src, ctx);

    // If the condition is true, jump from (instruction address + 2)

    if (i->condition->func(ctx))
    {
        ctx->pc = (ctx->instruction_address + 2 + (i->size == Byte ? (int8_t)offset : (int16_t)offset)) & M68K_ADDRESS_WIDTH;
        return 10;
    }

    return i->size == Byte ? 8 : 12;
}

void gen_bcc(Instruction* i, uint16_t opcode)
{
    Condition* condition = condition_get(FRAGMENT(opcode, 11, 8));

    // Format the instruction name depending on its internal condition

    char name[5];
    sprintf(name, "B%s", condition->mnemonics);

    instruction_init(i, name, bcc);
    i->condition = condition;

    int displacement = FRAGMENT(opcode, 7, 0);
    if (displacement == 0)
    {
        i->size = Word;
        operand_make_branching_offset(&i->src, i, Word);
    }
    else
    {
        i->size = Byte;
        operand_make_branching_offset(&i->src, i, Byte);
    }
}

uint8_t bra(Instruction* i, M68k* ctx)
{
    // Jump from (instruction address + 2)

    int16_t offset = FETCH_EA_AND_GET(&i->src, ctx);
    ctx->pc = (ctx->instruction_address + 2 + (i->size == Byte ? (int8_t)offset : (int16_t)offset)) & M68K_ADDRESS_WIDTH;

    return 0;
}

void gen_bra(Instruction* i, uint16_t opcode)
{
    instruction_init(i, "BRA", bra);
    i->base_cycles = 10;

    int offset = FRAGMENT(opcode, 7, 0);
    if (offset == 0)
    {
        i->size = Word;
        operand_make_branching_offset(&i->src, i, Word);
    }
    else
    {
        i->size = Byte;
        operand_make_branching_offset(&i->src, i, Byte);
    }
}

uint8_t bsr(Instruction* i, M68k* ctx)
{
    int16_t offset = FETCH_EA_AND_GET(&i->src, ctx);

    // Push the address that follows the instruction on the stack

    ctx->address_registers[7] -= 4;
    map_write_l(ctx->genesis->map, ctx->address_registers[7], ctx->pc);

    // Jump from (instruction address + 2)

    ctx->pc = (ctx->instruction_address + 2 + (i->size == Byte ? (int8_t)offset : (int16_t)offset)) & M68K_ADDRESS_WIDTH;

    return 0;
}

void gen_bsr(Instruction* i, uint16_t opcode)
{
    instruction_init(i, "BSR", bsr);
    i->base_cycles = 18;

    uint8_t offset = FRAGMENT(opcode, 7, 0);
    i->size = offset == 0 ? Word : Byte;
    operand_make_branching_offset(&i->src, i, i->size);
}

uint8_t dbcc(Instruction* i, M68k* ctx)
{
    int16_t offset = FETCH_EA_AND_GET(&i->dst, ctx);

    // If the condition is true, no operation is performed

    if (i->condition->func(ctx))
    {
        return 12;
    }

    bool branch_taken = false;

    // If the counter is still positive, jump from (instruction address + 2)

    uint16_t reg = GET(&i->src, ctx);
    if (reg > 0)
    {
        ctx->pc = (ctx->instruction_address + 2 + offset) & M68K_ADDRESS_WIDTH;
        branch_taken = true;
    }

    // Decrement the counter

    SET(&i->src, ctx, reg - 1);

    return branch_taken ? 10 : 14;
}

void gen_dbcc(Instruction* i, uint16_t opcode)
{
    Condition* condition = condition_get(FRAGMENT(opcode, 11, 8));

    // Format the instruction name depending on its internal condition

    char name[5];
    sprintf(name, "DB%s", condition->mnemonics);

    instruction_init(i, name, dbcc);
    i->size = Word;
    i->condition = condition;
    operand_make_data_register(&i->src, FRAGMENT(opcode, 2, 0), i);
    operand_make_branching_offset(&i->dst, i, Word);
}

uint8_t jmp(Instruction* i, M68k* ctx)
{
    ctx->pc = FETCH_EA(&i->src, ctx) & M68K_ADDRESS_WIDTH;
    ;

    return 0;
}

void gen_jmp(Instruction* i, uint16_t opcode)
{
    instruction_init(i, "JMP", jmp);
    operand_make(&i->src, FRAGMENT(opcode, 5, 0), i);
}

uint8_t jsr(Instruction* i, M68k* ctx)
{
    uint32_t ea = FETCH_EA(&i->src, ctx);

    if (i->src.type == AbsoluteShort)
    {
        ea = SIGN_EXTEND_W(ea);
    }

    // Push the address following the instruction onto the stack

    ctx->address_registers[7] -= 4;
    map_write_l(ctx->genesis->map, ctx->address_registers[7], ctx->pc);

    // Jump to the location specified by ea

    ctx->pc = ea & M68K_ADDRESS_WIDTH;

    return 0;
}

void gen_jsr(Instruction* i, uint16_t opcode)
{
    instruction_init(i, "JSR", jsr);
    operand_make(&i->src, FRAGMENT(opcode, 5, 0), i);
    i->size = Long;
}

uint8_t nop(Instruction* i, M68k* ctx)
{
    return 0;
}

void gen_nop(Instruction* i, uint16_t opcode)
{
    instruction_init(i, "NOP", nop);
    i->base_cycles = 4;
}

void gen_rtd(Instruction* i, uint16_t opcode)
{
    instruction_init(i, "RTD", not_implemented);
}

uint8_t rtr(Instruction* i, M68k* ctx)
{
    uint8_t ccr = map_read_w(ctx->genesis->map, ctx->address_registers[7]);
    ctx->status = (ctx->status & 0xFFE0) | (ccr & 0x1F);

    uint32_t pc = map_read_l(ctx->genesis->map, ctx->address_registers[7] + 2) & M68K_ADDRESS_WIDTH;
    ctx->pc = pc;

    ctx->address_registers[7] += 6;

    return 0;
}

void gen_rtr(Instruction* i, uint16_t opcode)
{
    instruction_init(i, "RTR", rtr);
    i->base_cycles = 20;
}

uint8_t rts(Instruction* i, M68k* ctx)
{
    ctx->pc = map_read_l(ctx->genesis->map, ctx->address_registers[7]) & M68K_ADDRESS_WIDTH;
    ctx->address_registers[7] += 4;

    return 0;
}

void gen_rts(Instruction* i, uint16_t opcode)
{
    instruction_init(i, "RTS", rts);
    i->base_cycles = 16;
}
