#include <stdbool.h>
#include <stdlib.h>

#include "bit_utils.h"
#include "conditions.h"
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
    if (direction == 0)
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
    FETCH_EAE(i->src);
    FETCH_EAE(i->dst);
    uint32_t result = GETE(i->src) & GETE(i->dst);
    SETE(i->dst, result);

    CARRY_SET(i->context, false);
    OVERFLOW_SET(i->context, false);
    ZERO_SET(i->context, result == 0);
    NEGATIVE_SET(i->context, BIT(result, i->size - 1));

    return 0;
}

Instruction* gen_and(uint16_t opcode, M68k* m)
{
    return gen_boolean_instruction(opcode, m, "AND", and);
}

Instruction* gen_andi(uint16_t opcode, M68k* m)
{
    return gen_boolean_instruction_immediate(opcode, m, "ANDI", and);
}

int eor(Instruction* i)
{
    FETCH_EAE(i->dst);
    uint32_t result = GETE(i->src) ^ GETE(i->dst);
    SETE(i->dst, result);

    CARRY_SET(i->context, false);
    OVERFLOW_SET(i->context, false);
    ZERO_SET(i->context, result == 0);
    NEGATIVE_SET(i->context, result < 0);

    return 0;
}

Instruction* gen_eor(uint16_t opcode, M68k* m)
{
    return gen_boolean_instruction(opcode, m, "EOR", eor);
}

Instruction* gen_eori(uint16_t opcode, M68k* m)
{
    return gen_boolean_instruction_immediate(opcode, m, "EORI", eor);
}

int or (Instruction* i)
{
    FETCH_EAE(i->dst);
    uint32_t result = GETE(i->src) | GETE(i->dst);
    SETE(i->dst, result);

    CARRY_SET(i->context, false);
    OVERFLOW_SET(i->context, false);
    ZERO_SET(i->context, result == 0);
    NEGATIVE_SET(i->context, result < 0);

    return 0;
}

Instruction* gen_or(uint16_t opcode, M68k* m)
{
    return gen_boolean_instruction(opcode, m, "OR", or );
}

Instruction* gen_ori(uint16_t opcode, M68k* m)
{
    return gen_boolean_instruction_immediate(opcode, m, "ORI", or );
}

int not(Instruction* i)
{
    FETCH_EAE(i->dst);
    SETE(i->dst, ~GETE(i->dst));

    uint32_t result = GETE(i->dst);
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
    return i;
}

int scc(Instruction* i)
{
    FETCH_EA_AND_SETE(i->dst, i->condition->func(i->context) ? 0xFF : 0);

    return 0;
}

Instruction* gen_scc(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "Scc", scc);
    i->size = Byte;
    i->condition = condition_get(FRAGMENT(opcode, 11, 8));
    i->dst = operand_make(FRAGMENT(opcode, 5, 0), i);
    return i;
}

int tst(Instruction* i)
{
    uint32_t value = FETCH_EA_AND_GETE(i->src);

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
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);
    return i;
}
