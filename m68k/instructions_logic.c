#include <stdbool.h>
#include <stdlib.h>

#include "bit_utils.h"
#include "conditions.h"
#include "cycles.h"
#include "instruction.h"
#include "m68k.h"
#include "operands.h"

Instruction* gen_boolean_instruction(uint16_t opcode, M68k* m, char* name, InstructionFunc* func)
{
    Instruction* i = instruction_make(m, name, func);
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

Instruction* gen_boolean_instruction_immediate(uint16_t opcode, M68k* m, char* name, InstructionFunc* func)
{
    Instruction* i = instruction_make(m, name, func);
    i->size = operand_size(FRAGMENT(opcode, 7, 6));
    i->src = operand_make_immediate_value(i->size, i);
    i->dst = operand_make(FRAGMENT(opcode, 5, 0), i);
    return i;
}

int and(Instruction* i)
{
    // Fetch both effective addresses to cover the two variants: AND ea, Dn & AND Dn, ea
    uint32_t result = FETCH_EA_AND_GET(i->src) & FETCH_EA_AND_GET(i->dst);
    SET(i->dst, result);

    CARRY_SET(i->context, false);
    OVERFLOW_SET(i->context, false);
    ZERO_SET(i->context, result == 0);
    NEGATIVE_SET(i->context, BIT(result, i->size - 1));

    return 0;
}

Instruction* gen_and(uint16_t opcode, M68k* m)
{
    Instruction* i = gen_boolean_instruction(opcode, m, "AND", and);

    if (instruction_is_valid(i, true, true))
        i->base_cycles = i->size == Long ?
            cycles_standard_instruction(i, 0, 4, 8) :
            cycles_standard_instruction(i, 0, 6, 12);

    return i;
}

Instruction* gen_andi(uint16_t opcode, M68k* m)
{
    return gen_boolean_instruction_immediate(opcode, m, "ANDI", and);
}

int andi_sr(Instruction* i)
{
    i->context->status &= FETCH_EA_AND_GET(i->src);

    return 0;
}

Instruction* gen_andi_sr(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "ANDI SR", andi_sr);
    i->src = operand_make_immediate_value(Byte, i);
    return i;
}

int andi_ccr(Instruction* i)
{
    i->context->status = (i->context->status & 0xFFE0) | (i->context->status & FETCH_EA_AND_GET(i->src) & 0x1F);

    return 0;
}

Instruction* gen_andi_ccr(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "ANDI CCR", andi_ccr);
    i->src = operand_make_immediate_value(Word, i);
    return i;
}

int eor(Instruction* i)
{
    uint32_t result = FETCH_EA_AND_GET(i->src) ^ FETCH_EA_AND_GET(i->dst);
    SET(i->dst, result);

    CARRY_SET(i->context, false);
    OVERFLOW_SET(i->context, false);
    ZERO_SET(i->context, result == 0);
    NEGATIVE_SET(i->context, result < 0);

    return 0;
}

Instruction* gen_eor(uint16_t opcode, M68k* m)
{
    Instruction* i = gen_boolean_instruction(opcode, m, "EOR", eor);

    if (instruction_is_valid(i, true, true))
        i->base_cycles = i->size == Long ?
            cycles_standard_instruction(i, 0, 4, 8) :
            cycles_standard_instruction(i, 0, 8, 12);

    return i;
}

Instruction* gen_eori(uint16_t opcode, M68k* m)
{
    return gen_boolean_instruction_immediate(opcode, m, "EORI", eor);
}

int or (Instruction* i)
{
    uint32_t result = FETCH_EA_AND_GET(i->src) | FETCH_EA_AND_GET(i->dst);
    SET(i->dst, result);

    CARRY_SET(i->context, false);
    OVERFLOW_SET(i->context, false);
    ZERO_SET(i->context, result == 0);
    NEGATIVE_SET(i->context, result < 0);

    return 0;
}

Instruction* gen_or(uint16_t opcode, M68k* m)
{
    Instruction* i = gen_boolean_instruction(opcode, m, "OR", or);

    if (instruction_is_valid(i, true, true))
        i->base_cycles = i->size == Long ?
            cycles_standard_instruction(i, 0, 4, 8) :
            cycles_standard_instruction(i, 0, 6, 12);
    
    return i;
}

Instruction* gen_ori(uint16_t opcode, M68k* m)
{
    return gen_boolean_instruction_immediate(opcode, m, "ORI", or);
}

int ori_sr(Instruction* i)
{
    i->context->status |= FETCH_EA_AND_GET(i->src);

    return 0;
}

Instruction* gen_ori_sr(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "ORI SR", ori_sr);
    i->src = operand_make_immediate_value(Byte, i);
    return i;
} // TODO cycles

int ori_ccr(Instruction* i)
{
    i->context->status = (i->context->status & 0xFFE0) | (i->context->status | FETCH_EA_AND_GET(i->src)) & 0x1F;

    return 0;
}

Instruction* gen_ori_ccr(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "ORI CCR", ori_ccr);
    i->src = operand_make_immediate_value(Word, i);
    return i;
} // TODO cycles

int not(Instruction* i)
{
    FETCH_EA(i->dst);
    SET(i->dst, ~GET(i->dst));

    uint32_t result = GET(i->dst);
    CARRY_SET(i->context, false);
    OVERFLOW_SET(i->context, false);
    ZERO_SET(i->context, result == 0);
    NEGATIVE_SET(i->context, BIT(result, i->size - 1) == 1);

    return 0;
}

Instruction* gen_not(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "NOT", not);
    i->size = operand_size(FRAGMENT(opcode, 7, 6));
    i->dst = operand_make(FRAGMENT(opcode, 5, 0), i);

    if (instruction_is_valid(i, false, true))
        i->base_cycles = i->size == Long ?
            cycles_single_operand_instruction(i, 6, 12) :
            cycles_single_operand_instruction(i, 4, 8);

    return i;
}

int scc(Instruction* i)
{
    FETCH_EA_AND_SET(i->dst, i->condition->func(i->context) ? 0xFF : 0);

    return 0;
}

Instruction* gen_scc(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "Scc", scc);
    i->size = Byte;
    i->condition = condition_get(FRAGMENT(opcode, 11, 8));
    i->dst = operand_make(FRAGMENT(opcode, 5, 0), i);

    if (instruction_is_valid(i, false, true))
        i->base_cycles = i->size == Long ?
            cycles_single_operand_instruction(i, 6, 8) :
            cycles_single_operand_instruction(i, 4, 8);

    return i;
}

int tst(Instruction* i)
{
    uint32_t value = FETCH_EA_AND_GET(i->dst);

    CARRY_SET(i->context, false);
    OVERFLOW_SET(i->context, false);
    ZERO_SET(i->context, value == 0);
    NEGATIVE_SET(i->context, BIT(value, i->size - 1) == 1);

    return 0;
}

Instruction* gen_tst(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "TST", tst);
    i->size = operand_size(FRAGMENT(opcode, 7, 6));
    i->dst = operand_make(FRAGMENT(opcode, 5, 0), i);

    if (instruction_is_valid(i, true, false))
        i->base_cycles = i->size == Long ?
            cycles_single_operand_instruction(i, 4, 4) :
            cycles_single_operand_instruction(i, 4, 4);

    return i;
}
