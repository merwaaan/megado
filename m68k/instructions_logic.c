#include <stdbool.h>
#include <stdlib.h>

#include "bit_utils.h"
#include "instruction.h"
#include "instructions_logic.h"
#include "operands.h"

Instruction* gen_logic_instruction(uint16_t opcode, M68k* m, char* name, InstructionFunc* func)
{
    Instruction* i = calloc(1, sizeof(Instruction));
    i->context = m;
    i->name = name;
    i->func = func;

    i->size = operand_size(FRAGMENT(opcode, 7, 6));

    Operand* op1 = operand_make_data(FRAGMENT(opcode, 11, 9), i);
    Operand* op2 = operand_make(FRAGMENT(opcode, 5, 0), i);
    int direction = BIT(opcode, 8);

    if (direction == 0)
    {
        i->src = op1;
        i->dst = op2;
    }
    else
    {
        i->src = op2;
        i->dst = op1;
    }

    return i;
}

void and(Instruction* i)
{
    int32_t initial = GET(i->dst);
    int32_t result = MASK_ABOVE_INC(GET(i->src) & initial, i->size);
    SET(i->dst, MASK_BELOW(initial, i->size) | result);
    
    CARRY_SET(i->context, false);
    OVERFLOW_SET(i->context, false);
    ZERO_SET(i->context, result == 0);
    NEGATIVE_SET(i->context, result < 0);
}

Instruction* gen_and(uint16_t opcode, M68k* m)
{
    return gen_logic_instruction(opcode, m, "AND", and);
}

void eor(Instruction* i)
{
    uint16_t dest = GET(i->dst);
    uint16_t result = GET(i->src) & dest;
    SET(i->dst, MASK_BELOW(dest, i->size) ^ MASK_ABOVE(result, i->size));

    CARRY_SET(i->context, false);
    OVERFLOW_SET(i->context, false);
    ZERO_SET(i->context, result == 0);
    NEGATIVE_SET(i->context, result < 0);
}

Instruction* gen_eor(uint16_t opcode, M68k* m)
{
    return gen_logic_instruction(opcode, m, "EOR", eor);
}

void or (Instruction* i)
{
    uint16_t dest = GET(i->dst);
    uint16_t result = GET(i->src) | dest;
    SET(i->dst, MASK_BELOW(dest, i->size) | MASK_ABOVE(result, i->size));

    CARRY_SET(i->context, false);
    OVERFLOW_SET(i->context, false);
    ZERO_SET(i->context, result == 0);
    NEGATIVE_SET(i->context, result < 0);
}

Instruction* gen_or(uint16_t opcode, M68k* m)
{
    return gen_logic_instruction(opcode, m, "OR", or );
}

void not(Instruction* i)
{
    int32_t initial = GET(i->src);
    int32_t result = MASK_ABOVE_INC(~initial, i->size);
    SET(i->src, MASK_BELOW(initial, i->size) | result);

    CARRY_SET(i->context, false);
    OVERFLOW_SET(i->context, false);
    ZERO_SET(i->context, result == 0);
    NEGATIVE_SET(i->context, result < 0);
}

Instruction* gen_not(uint16_t opcode, M68k* m)
{
    Instruction* i = calloc(1, sizeof(Instruction));
    i->context = m;
    i->name = "NOT";
    i->func = not;
    i->size = operand_size(FRAGMENT(opcode, 7, 6));
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);
    return i;
}

void tst(Instruction* i)
{
    uint16_t x = MASK_ABOVE_INC(GET(i->src), i->size);

    CARRY_SET(i->context, false);
    OVERFLOW_SET(i->context, false);
    ZERO_SET(i->context, x == 0);
    NEGATIVE_SET(i->context, x < 0); // TODO size masking
}

Instruction* gen_tst(uint16_t opcode, M68k* m)
{
    Instruction* i = calloc(1, sizeof(Instruction));
    i->context = m;
    i->name = "TST";
    i->func = tst;
    i->size = operand_size(FRAGMENT(opcode, 7, 6));
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);
    return i;
}
