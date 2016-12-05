#include <stdbool.h>
#include <stdlib.h>

#include "bit_utils.h"
#include "instruction.h"
#include "instructions_logic.h"
#include "operands.h"

void swap(Instruction* i)
{
    int32_t value = GET(i->operands[0]);
    uint16_t lo = value & 0xFFFF;
    uint16_t hi = value >> 16;
    int32_t result = (lo << 16) | hi;
    SET(i->operands[0], result);

    CARRY_SET(i->context, false);
    OVERFLOW_SET(i->context, false);
    ZERO_SET(i->context, result == 0);
    NEGATIVE_SET(i->context, BIT(result, 31));
}

Instruction* gen_swap(uint16_t opcode, M68k* m)
{
    Instruction* i = calloc(1, sizeof(Instruction));
    i->context = m;
    i->name = "SWAP";
    i->func = swap;

    i->operands = calloc(2, sizeof(Operand*));
    i->operands[0] = operand_make_data_register(FRAGMENT(opcode, 5, 0), i);
    i->operand_count = 1;

    return i;
}
