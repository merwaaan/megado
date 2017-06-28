#include <stdlib.h>

#include "cycles.h"
#include "instruction.h"
#include "m68k.h"
#include "operands.h"

int andi_sr(Instruction* i)
{
    i->context->status &= FETCH_EA_AND_GET(i->src);

    return 0;
}

Instruction* gen_andi_sr(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "ANDI SR", andi_sr);
    i->src = operand_make_immediate_value(Word, i);
    i->base_cycles = 20;
    return i;
}

int eori_sr(Instruction* i)
{
    i->context->status ^= FETCH_EA_AND_GET(i->src);

    return 0;
}

Instruction* gen_eori_sr(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "EORI SR", eori_sr);
    i->src = operand_make_immediate_value(Word, i);
    i->base_cycles = 20;
    return i;
}

int ori_sr(Instruction* i)
{
    i->context->status |= FETCH_EA_AND_GET(i->src);

    return 0;
}

Instruction* gen_ori_sr(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "ORI SR", ori_sr);
    i->src = operand_make_immediate_value(Word, i);
    i->base_cycles = 20;
    return i;
}

int move_from_sr(Instruction* i)
{
    FETCH_EA_AND_SET(i->dst, i->context->status);

    return 0;
}

Instruction* gen_move_from_sr(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "MOVE from SR", move_from_sr);
    i->size = Word;
    i->dst = operand_make(FRAGMENT(opcode, 5, 0), i);
    i->base_cycles = 6;
    return i;
}

int move_to_sr(Instruction* i)
{
    i->context->status = FETCH_EA_AND_GET(i->src);

    return 0;
}

Instruction* gen_move_to_sr(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "MOVE to SR", move_to_sr);
    i->size = Word;
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);
    i->base_cycles = 12;
    return i;
}

int move_usp(Instruction* i)
{
    // Register -> user stack pointer
    if (i->src != NULL)
        i->context->usp = GET(i->src);
    // User stack pointer -> register
    else
        SET(i->dst, i->context->usp);

    return 0;
}

Instruction* gen_move_usp(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "MOVE USP", move_usp);
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

Instruction* gen_reset(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "RESET", not_implemented);
    i->base_cycles = 132;
    return i;
}

int rte(Instruction* i)
{
    i->context->status = m68k_read_w(i->context, i->context->address_registers[7]);
    i->context->address_registers[7] += 2;
    i->context->pc = m68k_read_l(i->context, i->context->address_registers[7]);
    i->context->address_registers[7] += 4;

    return 0;
}

Instruction* gen_rte(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "RTE", rte);
    i->base_cycles = 20;
    return i;
}

Instruction* gen_stop(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "STOP", not_implemented);
    i->base_cycles = 4;
    return i;
}
