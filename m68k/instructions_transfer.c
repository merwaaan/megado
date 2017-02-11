#include <stdbool.h>
#include <stdlib.h>

#include "bit_utils.h"
#include "instruction.h"
#include "m68k.h"
#include "operands.h"

int exg(Instruction* i)
{
    int32_t dst = GET(i->dst);
    SET(i->dst, GET(i->src));
    SET(i->src, dst);

    return 0;
}

Instruction* gen_exg(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "EXG", exg);
    i->size = Long;
    i->base_cycles = 6;

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

int lea(Instruction* i)
{
    uint32_t ea = FETCH_EA(i->src);

    // TODO not documented but Regen does this, need to check other emulators
    if (i->src->type == AbsoluteShort)
        ea = SIGN_EXTEND_W(ea);

    SET(i->dst, ea);

    return 0;
}

Instruction* gen_lea(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "LEA", lea);
    i->size = Long;
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);
    i->dst = operand_make_address_register(FRAGMENT(opcode, 11, 9), i);
    return i;
}

int move_cycles_bw[12][9] =
{
    { 4, 4, 8, 8, 8, 12, 14, 12, 16 },
    { 4, 4, 8, 8, 8, 12, 14, 12, 16 },
    { 8, 8, 12, 12, 12, 16, 18, 16, 20 },
    { 8, 8, 12, 12, 12, 16, 18, 16, 20 },
    { 10, 10, 14, 14, 14, 18, 20, 18, 22 },
    { 12, 12, 16, 16, 16, 20, 22, 20, 24 },
    { 14, 14, 18, 18, 18, 22, 24, 22, 26 },
    { 12, 12, 16, 16, 16, 20, 22, 20, 24 },
    { 16, 16, 20, 20, 20, 24, 26, 24, 28 },
    { 12, 12, 16, 16, 16, 20, 22, 20, 24 },
    { 14, 14, 18, 18, 18, 22, 24, 22, 26 },
    { 8, 8, 12, 12, 12, 16, 18, 16, 20 }
};

int move_cycles_l[12][9] = // TODO fill this when you want to feel like a robot
{
    { 4, 4, 8, 8, 8, 12, 14, 12, 16 },
    { 4, 4, 8, 8, 8, 12, 14, 12, 16 },
    { 8, 8, 12, 12, 12, 16, 18, 16, 20 },
    { 8, 8, 12, 12, 12, 16, 18, 16, 20 },
    { 10, 10, 14, 14, 14, 18, 20, 18, 22 },
    { 12, 12, 16, 16, 16, 20, 22, 20, 24 },
    { 14, 14, 18, 18, 18, 22, 24, 22, 26 },
    { 12, 12, 16, 16, 16, 20, 22, 20, 24 },
    { 16, 16, 20, 20, 20, 24, 26, 24, 28 },
    { 12, 12, 16, 16, 16, 20, 22, 20, 24 },
    { 14, 14, 18, 18, 18, 22, 24, 22, 26 },
    { 8, 8, 12, 12, 12, 16, 18, 16, 20 }
};

int move(Instruction* i)
{
    uint32_t value = FETCH_EA_AND_GET(i->src);
    FETCH_EA_AND_SET(i->dst, value);

    CARRY_SET(i->context, false);
    OVERFLOW_SET(i->context, false);
    ZERO_SET(i->context, value == 0);
    NEGATIVE_SET(i->context, BIT(value, i->size - 1) == 1);

    return 0;
}

Instruction* gen_move(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "MOVE", move);
    i->size = operand_size2(FRAGMENT(opcode, 13, 12));
    i->base_cycles = (i->size == Long ? move_cycles_l : move_cycles_bw)[0][0]; // TODO
    i->dst = operand_make(FRAGMENT(opcode, 11, 9) | FRAGMENT(opcode, 8, 6) << 3, i);
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);
    return i;
}

int movem(Instruction* i)
{
    // TODO
    // incomplete version, this opcode is super confusing

    uint16_t mask = m68k_fetch(i->context);

    uint32_t cursor = i->context->address_registers[i->src->n] & 0xFFFFFF;

    int moved = 0;
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
                    i->src->post_func(i->src);
            }
            // register -> memory
            else
            {
                // TODO
            }

            cursor += i->size == 16 ? 2 : 4;
            ++moved;
        }

    // Revert the initial post-increment (hackish)
    if (i->src->type == AddressRegisterIndirectPostInc)
        i->context->address_registers[i->src->n] -= size_in_bytes(i->src->instruction->size);

    return 4 * moved;
}

Instruction* gen_movem(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "MOVEM", movem);
    i->size = operand_size3(BIT(opcode, 6));
    i->base_length = 4;

    Operand* ea = operand_make(FRAGMENT(opcode, 5, 0), i);

    int direction = BIT(opcode, 10);
    if (direction)
        i->src = ea;
    else
        i->dst = ea;

    return i;
}

int moveq(Instruction* i)
{
    int32_t value = SIGN_EXTEND_B_L(GET(i->src));
    SET(i->dst, value);

    CARRY_SET(i->context, false);
    OVERFLOW_SET(i->context, false);
    ZERO_SET(i->context, value == 0);
    NEGATIVE_SET(i->context, BIT(value, 31) == 1);

    return 0;
}

Instruction* gen_moveq(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "MOVEQ", moveq);
    i->size = Long;
    i->src = operand_make_value(BYTE_LO(opcode), i);
    i->dst = operand_make_data_register(FRAGMENT(opcode, 11, 9), i);
    return i;
}

int movea(Instruction* i)
{
    int32_t value = FETCH_EA_AND_GET(i->src);

    if (i->size == Word)
        value = SIGN_EXTEND_W(value);

    SET(i->dst, value);

    return 0;
}

Instruction* gen_movea(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "MOVEA", movea);
    i->size = operand_size2(FRAGMENT(opcode, 13, 12));
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);
    i->dst = operand_make_address_register(FRAGMENT(opcode, 11, 9), i);
    return i;
}

int move_from_ccr(Instruction* i)
{
    // Only move the CCR segment of the status register
    FETCH_EA_AND_SET(i->dst, i->context->status & 0x1F);

    return 0;
}

Instruction* gen_move_from_ccr(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "MOVE from CCR", move_from_ccr);
    i->size = Word;
    i->dst = operand_make(FRAGMENT(opcode, 5, 0), i);
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
    return i;
}

int move_to_ccr(Instruction* i)
{
    // Only update the CCR segment of the status register
    i->context->status = i->context->status & 0xFFE0 | FETCH_EA_AND_GET(i->src) & 0x1F;

    return 0;
}

Instruction* gen_move_to_ccr(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "MOVE to CCR", move_to_ccr);
    i->size = Word;
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);
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
    return i;
}

int move_usp(Instruction* i)
{
    // Register to stack pointer
    if (i->src != NULL)
        i->context->address_registers[7] = GET(i->src);
    // Stack pointer to register
    else
        SET(i->dst, i->context->address_registers[7]);

    return 0;
}

Instruction* gen_move_usp(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "MOVE USP", move_usp);
    i->size = Long;

    Operand* reg = operand_make_address_register_indirect(FRAGMENT(opcode, 2, 0), i);

    int direction = BIT(opcode, 3);
    if (direction)
        i->dst = reg;
    else
        i->src = reg;

    return i;
}

int pea(Instruction* i)
{
    // TODO
    //i->context->memory[i->context->address_registers[7]] = GET(i->src);
    //i->context->address_registers[7]--;

    return 0;
}

Instruction* gen_pea(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "PEA", pea);
    i->dst = operand_make(FRAGMENT(opcode, 5, 0), i);
    return i;
}
