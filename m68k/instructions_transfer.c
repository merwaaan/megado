#include <stdbool.h>
#include <stdlib.h>

#include "bit_utils.h"
#include "instruction.h"
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
    i->size = Long;

    int mode = FRAGMENT(opcode, 7, 3);
    switch (mode)
    {
    case 8:
        i->src = operand_make_data_register(FRAGMENT(opcode, 11, 9), i);
        i->dst = operand_make_data_register(FRAGMENT(opcode, 2, 0), i);
        break;
    case 9:
        i->src = operand_make_address_register(FRAGMENT(opcode, 11, 9), i);
        i->dst = operand_make_address_register(FRAGMENT(opcode, 2, 0), i);
        break;
    case 17:
        i->src = operand_make_data_register(FRAGMENT(opcode, 11, 9), i);
        i->dst = operand_make_address_register(FRAGMENT(opcode, 2, 0), i);
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
    i->size = Long;
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);
    i->dst = operand_make_address_register(FRAGMENT(opcode, 11, 9), i);
    return i;
}

void move(Instruction* i)
{
    int32_t value = GET(i->src);
    SET(i->dst, value);

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
    i->dst = operand_make(FRAGMENT(opcode, 11, 9) | FRAGMENT(opcode, 8, 6) << 3, i);
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);
    return i;
}

void movem(Instruction* i)
{
    // TODO
    // incomplete version, this opcode is super confusing

    uint16_t mask = m68k_read_w(i->context, i->context->pc + 2);

    uint32_t cursor = i->context->address_registers[i->src->n] & 0xFFFFFF;

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
                if (i->src->type == AddressRegisterIndirectPostInc)
                    i->src->post(i->src);
            }
            // register -> memory
            else
            {
                // TODO
            }

            cursor += i->size == 16 ? 2 : 4;
        }

    // Revert the initial post-increment (hackish)
    if (i->src->type == AddressRegisterIndirectPostInc)
        i->context->address_registers[i->src->n] -= size_in_bytes(i->src->instruction->size);
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

void moveq(Instruction* i)
{
    int32_t value = SIGN_EXTEND_B_L(GET(i->src));
    SET(i->dst, value);

    CARRY_SET(i->context, false);
    OVERFLOW_SET(i->context, false);
    ZERO_SET(i->context, value == 0);
    NEGATIVE_SET(i->context, BIT(value, 31) == 1);
}

Instruction* gen_moveq(uint16_t opcode, M68k* m)
{
    Instruction* i = calloc(1, sizeof(Instruction));
    i->context = m;
    i->name = "MOVEQ";
    i->func = moveq;
    i->size = Long;
    i->src = operand_make_value(BYTE_LO(opcode), i);
    i->dst = operand_make_data_register(FRAGMENT(opcode, 11, 9), i);
    return i;
}

void movea(Instruction* i)
{
    int32_t value = GET(i->src);

    if (i->size == Word)
        value = SIGN_EXTEND_W(value);

    SET(i->dst, value);
}

Instruction* gen_movea(uint16_t opcode, M68k* m)
{
    Instruction* i = calloc(1, sizeof(Instruction));
    i->context = m;
    i->name = "MOVEA";
    i->func = movea;
    i->size = operand_size2(FRAGMENT(opcode, 13, 12));
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);
    i->dst = operand_make_address_register(FRAGMENT(opcode, 11, 9), i);
    return i;
}

void move_from_sr(Instruction* i)
{
    SET(i->dst, i->context->status);
}

Instruction* gen_move_from_sr(uint16_t opcode, M68k* m)
{
    Instruction* i = calloc(1, sizeof(Instruction));
    i->context = m;
    i->name = "MOVE from SR";
    i->func = move_from_sr;
    i->size = Word;
    i->dst = operand_make(FRAGMENT(opcode, 5, 0), i);
    return i;
}

void move_to_sr(Instruction* i)
{
    uint16_t value = GET(i->src);
    i->context->status = value;

    CARRY_SET(i->context, BIT(value, 0));
    OVERFLOW_SET(i->context, BIT(value, 1));
    ZERO_SET(i->context, BIT(value, 2));
    NEGATIVE_SET(i->context, BIT(value, 3));
    EXTENDED_SET(i->context, BIT(value, 4));
}

Instruction* gen_move_to_sr(uint16_t opcode, M68k* m)
{
    Instruction* i = calloc(1, sizeof(Instruction));
    i->context = m;
    i->name = "MOVE to SR";
    i->func = move_to_sr;
    i->size = Word;
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);
    return i;
}

void move_usp(Instruction* i)
{
    // Register to stack pointer
    if (i->src != NULL)
        i->context->address_registers[7] = GET(i->src);
    // Stack pointer to register
    else
        SET(i->dst, i->context->address_registers[7]);
}

Instruction* gen_move_usp(uint16_t opcode, M68k* m)
{
    Instruction* i = calloc(1, sizeof(Instruction));
    i->context = m;
    i->name = "MOVE USP";
    i->func = move_usp;
    i->size = Long;

    Operand* reg = operand_make_address_register_indirect(FRAGMENT(opcode, 2, 0), i);

    int direction = BIT(opcode, 3);
    if (direction)
        i->dst = reg;
    else
        i->src = reg;

    return i;
}

void pea(Instruction* i)
{
    // TODO
    //i->context->memory[i->context->address_registers[7]] = GET(i->src);
    //i->context->address_registers[7]--;
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
