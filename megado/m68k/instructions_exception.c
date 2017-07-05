#include <stdlib.h>

#include "cycles.h"
#include "instruction.h"
#include "m68k.h"
#include "operands.h"

Instruction* gen_chk(uint16_t opcode)
{
    Instruction* i = instruction_make("CHK", not_implemented);
    return i;
}

Instruction* gen_illegal(uint16_t opcode)
{
    Instruction* i = instruction_make("ILLEGAL", not_implemented);
    return i;
}

uint8_t trap(Instruction* i, M68k* ctx)
{
    // Push the current PC onto the stack
    ctx->address_registers[7] -= 4;
    m68k_write_l(ctx, ctx->address_registers[7], ctx->pc);

    // Push the status register onto the stack
    ctx->address_registers[7] -= 2;
    m68k_write_w(ctx, ctx->address_registers[7], ctx->status);

    ctx->pc = m68k_read_l(ctx, 0x80 + i->src->n * 4);

    return 0;
}

Instruction* gen_trap(uint16_t opcode)
{
    Instruction* i = instruction_make("TRAP", trap);
    i->src = operand_make_value(FRAGMENT(opcode, 3, 0), i);
    return i;
}

Instruction* gen_trapv(uint16_t opcode)
{
    Instruction* i = instruction_make("TRAPV", not_implemented);
    i->base_cycles = 4;
    return i;
}
