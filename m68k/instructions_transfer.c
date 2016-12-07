#include <stdbool.h>
#include <stdlib.h>

#include "bit_utils.h"
#include "instruction.h"
#include "instructions_logic.h"
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

    i->operands = calloc(2, sizeof(Operand*));
    i->operand_count = 2;

    int mode = FRAGMENT(opcode, 7, 3);
    switch (mode)
    {
    case 8:
        i->operands[0] = operand_make_data_register(FRAGMENT(opcode, 11, 9), i);
        i->operands[1] = operand_make_data_register(FRAGMENT(opcode, 2, 0), i);
        break;
    case 9:
        i->operands[0] = operand_make_address_register(FRAGMENT(opcode, 11, 9), i);
        i->operands[1] = operand_make_address_register(FRAGMENT(opcode, 2, 0), i);
        break;
    case 17:
        i->operands[0] = operand_make_data_register(FRAGMENT(opcode, 11, 9), i);
        i->operands[1] = operand_make_address_register(FRAGMENT(opcode, 2, 0), i);
        break;
    default:
        // TODO error
        break;
    }

    return i;
}
