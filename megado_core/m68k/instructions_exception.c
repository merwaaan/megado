#include "../genesis.h"
#include "instruction.h"
#include "m68k.h"
#include "map.h"
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
    map_write_l(ctx->genesis->map, ctx->address_registers[7], ctx->pc);

    // Push the status register onto the stack

    ctx->address_registers[7] -= 2;
    map_write_w(ctx->genesis->map, ctx->address_registers[7], ctx->status);

    ctx->pc = map_read_l(ctx->genesis->map, 0x80 + i->src->n * 4) & M68K_ADDRESS_WIDTH;

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
