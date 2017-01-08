#include <stdbool.h>
#include <stdlib.h>

#include "bit_utils.h"
#include "instruction.h"
#include "instructions_logic.h"
#include "m68k.h"
#include "operands.h"

void exg(Instruction* i)
{
    int32_t dst = GET(i->dst);
    SET(i->dst, GET(i->src));
    SET(i->src, dst);
}

Instruction* gen_exg(uint16_t opcode, M68k* m)
{
    Instruction* i = calloc(1, sizeof(Instruction));
    i->context = m;
    i->name = "EXG";
    i->func = exg;

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
    int32_t value = MASK_ABOVE(GET(i->src), i->size);
    SET(i->dst, MASK_BELOW_INC(GET(i->dst), i->size) | value);

    CARRY_SET(i->context, false);
    OVERFLOW_SET(i->context, false);
    ZERO_SET(i->context, value == 0);
    NEGATIVE_SET(i->context, BIT(value, i->size - 1) == 1);
}

Instruction* gen_move(uint16_t opcode, M68k* m)
{
    Instruction* i = calloc(1, sizeof(Instruction));
    i->context = m;
    i->name = "MOVE";
    i->func = move;
    i->size = operand_size2(FRAGMENT(opcode, 13, 12));

    i->dst = operand_make(FRAGMENT(opcode, 11, 6), i);
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);

    return i;
}

void movem(Instruction* i)
{
    // TODO
    // incomplete version, this opcode is super confusing

    uint16_t mask = m68k_read_w(i->context, i->context->pc + 2);

    uint32_t cursor = i->context->address_registers[i->src->n];

    for (int m = 0; m < 16; ++m)
        if (BIT(mask, m))
        {
            // memory -> register
            if (i->src != NULL)
            {
                uint32_t* reg = m < 8 ? i->context->data_registers + m : i->context->address_registers + m - 8;

                *reg = m68k_read(i->context, i->size, cursor);

                if (i->size == Word)
                    *reg = SIGN_EXTEND_W(*reg);

                // In post-increment mode, increment after EACH transfer
                if (i->src->type == AddressIndirectPostInc)
                    i->src->post(i->src);
            }
            // register -> memory
            else
            {
                // TODO
            }

            cursor += i->size == 16 ? 2 : 4;
        }
}

Instruction* gen_movem(uint16_t opcode, M68k* m)
{
    Instruction* i = calloc(1, sizeof(Instruction));
    i->context = m;
    i->name = "MOVEM";
    i->func = movem;
    i->size = operand_size3(BIT(opcode, 6));
    i->length += 2;

    Operand* ea = operand_make(FRAGMENT(opcode, 5, 0), i);

    int direction = BIT(opcode, 10);
    if (direction)
        i->src = ea;
    else
        i->dst = ea;

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
