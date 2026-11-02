#include "../genesis.h"
#include "instruction.h"
#include "m68k.h"
#include "map.h"
#include "operands.h"

void gen_chk(Instruction* i, uint16_t opcode)
{
    instruction_init(i, "CHK", not_implemented);
}

void gen_illegal(Instruction* i, uint16_t opcode)
{
    instruction_init(i, "ILLEGAL", not_implemented);
}

uint8_t trap(Instruction* i, M68k* ctx)
{
    // Push the current PC onto the stack

    ctx->address_registers[7] -= 4;
    map_write_l(ctx->genesis->map, ctx->address_registers[7], ctx->pc);

    // Push the status register onto the stack

    ctx->address_registers[7] -= 2;
    map_write_w(ctx->genesis->map, ctx->address_registers[7], ctx->status);

    ctx->pc = map_read_l(ctx->genesis->map, 0x80 + i->src.n * 4) & M68K_ADDRESS_WIDTH;

    return 0;
}

void gen_trap(Instruction* i, uint16_t opcode)
{
    instruction_init(i, "TRAP", trap);
    operand_make_value(&i->src, FRAGMENT(opcode, 3, 0), i);
}

void gen_trapv(Instruction* i, uint16_t opcode)
{
    instruction_init(i, "TRAPV", not_implemented);
    i->base_cycles = 4;
}
