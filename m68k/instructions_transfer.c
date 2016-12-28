#include <stdbool.h>
#include <stdlib.h>

#include "bit_utils.h"
#include "instruction.h"
#include "instructions_logic.h"
#include "m68k.h"
#include "operands.h"

void exg(Instruction* i)
{
    int32_t op1_value = GET(i->operands[1]);
    SET(i->operands[1], GET(i->operands[0]));
    SET(i->operands[0], op1_value);
}

Instruction* gen_exg(uint16_t opcode, M68k* m)
{
    Instruction* i = calloc(1, sizeof(Instruction));
    i->context = m;
    i->name = "EXG";
    i->func = exg;

    i->size = operand_size(FRAGMENT(opcode, 7, 6));

    i->operands = calloc(2, sizeof(Operand));
    i->operand_count = 2;

    int mode = FRAGMENT(opcode, 7, 3);
    switch (mode)
    {
    case 8:
        i->operands[0] = operand_make_data(FRAGMENT(opcode, 11, 9), i);
        i->operands[1] = operand_make_data(FRAGMENT(opcode, 2, 0), i);
        break;
    case 9:
        i->operands[0] = operand_make_address(FRAGMENT(opcode, 11, 9), i);
        i->operands[1] = operand_make_address(FRAGMENT(opcode, 2, 0), i);
        break;
    case 17:
        i->operands[0] = operand_make_data(FRAGMENT(opcode, 11, 9), i);
        i->operands[1] = operand_make_address(FRAGMENT(opcode, 2, 0), i);
        break;
    default:
        // TODO error
        break;
    }

    return i;
}

void lea(Instruction* i)
{
    SET(i->operands[1], GET(i->operands[0]));
}

Instruction* gen_lea(uint16_t opcode, M68k* m)
{
    Instruction* i = calloc(1, sizeof(Instruction));
    i->context = m;
    i->name = "LEA";
    i->func = lea;

    i->operands = calloc(2, sizeof(Operand));
    i->operands[0] = operand_make_address(FRAGMENT(opcode, 11, 9), i);
    i->operands[1] = operand_make(FRAGMENT(opcode, 5, 0), i);
    i->operand_count = 2;

    return i;
}

void move(Instruction* i)
{
    int32_t moved = MASK_ABOVE(GET(i->operands[0]), i->size);
    SET(i->operands[1], MASK_BELOW_INC(GET(i->operands[1]), i->size) | moved);

    CARRY_SET(i->context, false);
    OVERFLOW_SET(i->context, false);
    ZERO_SET(i->context, moved == 0);
    NEGATIVE_SET(i->context, moved < 0);
}

Instruction* gen_move(uint16_t opcode, M68k* m)
{
    Instruction* i = calloc(1, sizeof(Instruction));
    i->context = m;
    i->name = "MOVE";
    i->func = move;

    i->operands = calloc(1, sizeof(Operand));
    i->operands[1] = operand_make(FRAGMENT(opcode, 5, 0), i);
    i->operand_count = 2;

    return i;
}

void pea(Instruction* i)
{
    i->context->memory[i->context->address_registers[7]] = GET(i->operands[0]);
    i->context->address_registers[7]--;
}

Instruction* gen_pea(uint16_t opcode, M68k* m)
{
    Instruction* i = calloc(1, sizeof(Instruction));
    i->context = m;
    i->name = "PEA";
    i->func = pea;

    i->operands = calloc(1, sizeof(Operand));
    i->operands[1] = operand_make(FRAGMENT(opcode, 5, 0), i);
    i->operand_count = 2;

    return i;
}
