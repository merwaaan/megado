#include <stdlib.h>

#include "cycles.h"
#include "instruction.h"
#include "m68k.h"
#include "operands.h"

Instruction* gen_chk(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "CHK", not_implemented);
    return i;
}

Instruction* gen_illegal(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "ILLEGAL", not_implemented);
    return i;
}

int trap(Instruction* i)
{
    // Push the current PC onto the stack
    i->context->address_registers[7] -= 4;
    m68k_write_l(i->context, i->context->address_registers[7], i->context->pc);

    // Push the status register onto the stack
    i->context->address_registers[7] -= 2;
    m68k_write_w(i->context, i->context->address_registers[7], i->context->status);

    i->context->pc = m68k_read_l(i->context, 0x80 + i->src->n * 4);

    return 0;
}

Instruction* gen_trap(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "TRAP", trap);
    i->src = operand_make_value(FRAGMENT(opcode, 3, 0), i);
    return i;
}

Instruction* gen_trapv(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "TRAPV", not_implemented);
    i->base_cycles = 4;
    return i;
}
