#include <stdbool.h>
#include <stdlib.h>

#include "bit_utils.h"
#include "cycles.h"
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

int link(Instruction* i)
{
    // Push the address register onto the stack
    i->context->address_registers[7] -= 4;
    m68k_write_l(i->context, i->context->address_registers[7], i->context->address_registers[i->src->n]);

    // Place the new stack pointer in the address register
    i->context->address_registers[i->src->n] = i->context->address_registers[7];

    // Add the offset to the stack pointer
    i->context->address_registers[7] += (int16_t)FETCH_EA_AND_GET(i->dst);

    return 0;
}

Instruction* gen_link(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "LINK", link);
    i->size = Long;
    i->src = operand_make_address_register(FRAGMENT(opcode, 2, 0), i);
    i->dst = operand_make_immediate_value(Word, i);
    return i;
}

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
    i->dst = operand_make(FRAGMENT(opcode, 11, 9) | FRAGMENT(opcode, 8, 6) << 3, i);
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);

    i->base_cycles = cycles_move_table[i->size == Long][i->src->type][i->dst->type];

    return i;
}

// Computer pointers to the nth register in post-inc or pre-dec order
#define MOVEM_POSTINC_ORDER(n) (n < 8 ? i->context->data_registers + n : i->context->address_registers + n - 8)
#define MOVEM_PREDEC_ORDER(n) (n < 8 ? i->context->address_registers + (7 - n) : i->context->data_registers + (7 - (n - 8)))

int movem(Instruction* i)
{
    // TODO (refactor)
    // incomplete version, this opcode is super confusing

    uint16_t mask = m68k_fetch(i->context);

    Operand* ea = i->src != NULL ? i->src : i->dst;
    uint32_t offset = FETCH_EA(ea);

    int moved = 0;
    for (int m = 0; m < 16; ++m)
        if (BIT(mask, m))
        {
            // memory -> register
            if (i->src != NULL)
            {
                uint32_t* reg = ea->type == AddressRegisterIndirectPreDec ? MOVEM_PREDEC_ORDER(m) : MOVEM_POSTINC_ORDER(m);

                *reg = m68k_read(i->context, i->size, offset);

                if (i->size == Word)
                    *reg = SIGN_EXTEND_W(*reg);
            }
            // register -> memory
            else
            {
                uint32_t reg = *(ea->type == AddressRegisterIndirectPreDec ? MOVEM_PREDEC_ORDER(m) : MOVEM_POSTINC_ORDER(m));

                if (i->size == Word)
                    reg = SIGN_EXTEND_W(reg);

                m68k_write(i->context, i->size, offset, reg);
            }

            if (ea->type == AddressRegisterIndirectPreDec)
                offset -= size_in_bytes(i->size);
            else
                offset += size_in_bytes(i->size);

            ++moved;
        }

    // Update the address register in pre-dec/post-inc modes
    // (take into account the one dec/inc that is handled by the operand's pre/post functions) 
    if (ea->type == AddressRegisterIndirectPreDec)
        i->context->address_registers[ea->n] = offset + size_in_bytes(i->size);
    else if (ea->type == AddressRegisterIndirectPostInc)
        i->context->address_registers[ea->n] = offset - size_in_bytes(i->size);

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

    i->context->address_registers[i->dst->n] = value;

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
    // Register -> user stack pointer
    if (i->src != NULL)
        i->context->usp = GET(i->src);
    // User stack pointer -> register
    else
        SET(i->dst, i->context->usp);

    return 0;
}

Instruction* gen_move_usp(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "MOVE USP", move_usp);
    i->size = Long;

    Operand* reg = operand_make_address_register(FRAGMENT(opcode, 2, 0), i);

    int direction = BIT(opcode, 3);
    if (direction)
        i->dst = reg;
    else
        i->src = reg;

    return i;
}

int pea(Instruction* i)
{
    i->context->address_registers[7] -= 4;
    m68k_write_l(i->context, i->context->address_registers[7], FETCH_EA(i->src));

    return 0;
}

Instruction* gen_pea(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "PEA", pea);
    i->size = Long;
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);
    return i;
}

int unlk(Instruction* i)
{
    // Load the stack pointer with the address register
    i->context->address_registers[7] = i->context->address_registers[i->src->n];
    
    // Load the address register with the long word at the top of the stack
    i->context->address_registers[i->src->n] = m68k_read_l(i->context, i->context->address_registers[7]);
    i->context->address_registers[7] += 4;

    return 0;
}

Instruction* gen_unlk(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "UNLK", unlk);
    i->size = Long;
    i->src = operand_make_address_register(FRAGMENT(opcode, 2, 0), i);
    return i;
}
