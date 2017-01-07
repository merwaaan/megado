#include <stdbool.h>
#include <stdlib.h>

#include "bit_utils.h"
#include "instruction.h"
#include "instructions_logic.h"
#include "m68k.h"
#include "operands.h"

void exg(Instruction* i)
{
    int32_t op1_value = GET(i->dst);
    SET(i->dst, GET(i->src));
    SET(i->src, op1_value);
}

Instruction* gen_exg(uint16_t opcode, M68k* m)
{
    Instruction* i = calloc(1, sizeof(Instruction));
    i->context = m;
    i->name = "EXG";
    i->func = exg;

    i->size = operand_size(FRAGMENT(opcode, 7, 6));

    int mode = FRAGMENT(opcode, 7, 3);
    switch (mode)
    {
    case 8:
        i->src = operand_make_data(FRAGMENT(opcode, 11, 9), i);
        i->dst = operand_make_data(FRAGMENT(opcode, 2, 0), i);
        break;
    case 9:
        i->src = operand_make_address(FRAGMENT(opcode, 11, 9), i);
        i->dst = operand_make_address(FRAGMENT(opcode, 2, 0), i);
        break;
    case 17:
        i->src = operand_make_data(FRAGMENT(opcode, 11, 9), i);
        i->dst = operand_make_address(FRAGMENT(opcode, 2, 0), i);
        break;
    default:
        // TODO error
        break;
    }

    return i;
}

void lea(Instruction* i)
{
    SET(i->dst, GET(i->src));
}

Instruction* gen_lea(uint16_t opcode, M68k* m)
{
    Instruction* i = calloc(1, sizeof(Instruction));
    i->context = m;
    i->name = "LEA";
    i->func = lea;

    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);
    i->dst = operand_make_address(FRAGMENT(opcode, 11, 9), i);

    return i;
}

void move(Instruction* i)
{
    int32_t moved = MASK_ABOVE(GET(i->src), i->size);
    SET(i->dst, MASK_BELOW_INC(GET(i->dst), i->size) | moved);

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

    i->dst = operand_make(FRAGMENT(opcode, 5, 0), i);

    return i;
}

void movem(Instruction* i)
{
    uint16_t mask = (i->context->memory[i->context->pc + 1] << 8) | i->context->memory[i->context->pc + 2];

    // TODO different mask for predecrement mode
    // TODO long size
    // TODO sign extend words

    uint32_t cursor = GET(i->src ? i->src : i->dst);

    for (int m = 0; m < 16; ++m)
        if (mask & (1 << (15 - m))) {

            // memory -> register
            if (i->src != NULL)
                i->context->data_registers[m] = i->context->memory[cursor];
            // register -> memory
            else
                i->context->memory[cursor] = i->context->data_registers[m];

            cursor += i->size == 16 ? 2 : 4;
        }
}

Instruction* gen_movem(uint16_t opcode, M68k* m)
{
    Instruction* i = calloc(1, sizeof(Instruction));
    i->context = m;
    i->name = "MOVEM";
    i->func = movem;
    i->size = operand_size2(BIT(opcode, 6));

    Operand* ea = operand_make(FRAGMENT(opcode, 5, 0), i);

    int direction = BIT(opcode, 10);
    if (direction)
        i->dst = ea;
    else
        i->src = ea;

    return i;
}

void pea(Instruction* i)
{
    i->context->memory[i->context->address_registers[7]] = GET(i->src);
    i->context->address_registers[7]--;
}

Instruction* gen_pea(uint16_t opcode, M68k* m)
{
    Instruction* i = calloc(1, sizeof(Instruction));
    i->context = m;
    i->name = "PEA";
    i->func = pea;

    i->dst = operand_make(FRAGMENT(opcode, 5, 0), i);

    return i;
}
