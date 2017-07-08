#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "bit_utils.h"
#include "cycles.h"
#include "instruction.h"
#include "m68k.h"
#include "operands.h"

uint8_t exg(Instruction* i, M68k* ctx)
{
    int32_t dst = GET(i->dst, ctx);
    SET(i->dst, ctx, GET(i->src, ctx));
    SET(i->src, ctx, dst);

    return 0;
}

Instruction* gen_exg(uint16_t opcode)
{
    Instruction* i = instruction_make("EXG", exg);
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
        fprintf(stderr, "Invalid mode %x in gen_exg\n", mode);
        exit(1);
        break;
    }

    i->base_cycles = 6;

    return i;
}

uint8_t lea(Instruction* i, M68k* ctx)
{
    uint32_t ea = FETCH_EA(i->src, ctx);

    // TODO not documented but Regen does this, need to check other emulators
    if (i->src->type == AbsoluteShort)
        ea = SIGN_EXTEND_W(ea);

    SET(i->dst, ctx, ea);

    return 0;
}

Instruction* gen_lea(uint16_t opcode)
{
    Instruction* i = instruction_make("LEA", lea);
    i->size = Long;
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);
    i->dst = operand_make_address_register(FRAGMENT(opcode, 11, 9), i);
    return i;
}

uint8_t link(Instruction* i, M68k* ctx)
{
    // Push the address register onto the stack
    ctx->address_registers[7] -= 4;
    m68k_write_l(ctx, ctx->address_registers[7], ctx->address_registers[i->src->n]);

    // Place the new stack pointer in the address register
    ctx->address_registers[i->src->n] = ctx->address_registers[7];

    // Add the offset to the stack pointer
    ctx->address_registers[7] += (int16_t)FETCH_EA_AND_GET(i->dst, ctx);

    return 0;
}

Instruction* gen_link(uint16_t opcode)
{
    Instruction* i = instruction_make("LINK", link);
    i->size = Long;
    i->src = operand_make_address_register(FRAGMENT(opcode, 2, 0), i);
    i->dst = operand_make_immediate_value(Word, i);
    i->base_cycles = 16;
    return i;
}

uint8_t move(Instruction* i, M68k* ctx)
{
    uint32_t value = FETCH_EA_AND_GET(i->src, ctx);
    FETCH_EA_AND_SET(i->dst, ctx, value);

    CARRY_SET(ctx, false);
    OVERFLOW_SET(ctx, false);
    ZERO_SET(ctx, value == 0);
    NEGATIVE_SET(ctx, BIT(value, i->size - 1) == 1);

    return 0;
}

Instruction* gen_move(uint16_t opcode)
{
    Instruction* i = instruction_make("MOVE", move);
    i->size = operand_size2(FRAGMENT(opcode, 13, 12));
    i->dst = operand_make(FRAGMENT(opcode, 11, 9) | FRAGMENT(opcode, 8, 6) << 3, i);
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);
    i->base_cycles = cycles_move_table[i->size == Long][i->src->type][i->dst->type];
    return i;
}

// Computer pointers to the nth register in post-inc or pre-dec order
#define MOVEM_POSTINC_ORDER(n) (n < 8 ? (uint32_t*)ctx->data_registers + n : ctx->address_registers + (n - 8))
#define MOVEM_PREDEC_ORDER(n) (n < 8 ? ctx->address_registers + (7 - n) : (uint32_t*)ctx->data_registers + (7 - (n - 8)))

uint8_t movem(Instruction* i, M68k* ctx)
{
    // TODO mask as an operand

    uint16_t mask = m68k_fetch(ctx);

    Operand* ea = i->src != NULL ? i->src : i->dst;
    uint32_t offset = FETCH_EA(ea, ctx);

    int moved = 0;
    for (int m = 0; m < 16; ++m)
        if (BIT(mask, m))
        {
            // memory -> register
            if (i->src != NULL)
            {
                uint32_t* reg = ea->type == AddressRegisterIndirectPreDec ? MOVEM_PREDEC_ORDER(m) : MOVEM_POSTINC_ORDER(m);

                *reg = m68k_read(ctx, i->size, offset);

                if (i->size == Word)
                    *reg = SIGN_EXTEND_W(*reg);
            }
            // register -> memory
            else
            {
                uint32_t* reg = ea->type == AddressRegisterIndirectPreDec ? MOVEM_PREDEC_ORDER(m) : MOVEM_POSTINC_ORDER(m);

                if (i->size == Word)
                    *reg = SIGN_EXTEND_W(*reg);

                m68k_write(ctx, i->size, offset, *reg);
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
        ctx->address_registers[ea->n] = offset + size_in_bytes(i->size);
    else if (ea->type == AddressRegisterIndirectPostInc)
        ctx->address_registers[ea->n] = offset - size_in_bytes(i->size);

    return 4 * moved;
}

Instruction* gen_movem(uint16_t opcode)
{
    Instruction* i = instruction_make("MOVEM", movem);
    i->size = operand_size3(BIT(opcode, 6));

    Operand* ea = operand_make(FRAGMENT(opcode, 5, 0), i);

    int direction = BIT(opcode, 10);
    if (direction)
        i->src = ea;
    else
        i->dst = ea;

    return i;
}

uint8_t moveq(Instruction* i, M68k* ctx)
{
    int32_t value = SIGN_EXTEND_B_L(GET(i->src, ctx));
    SET(i->dst, ctx, value);

    CARRY_SET(ctx, false);
    OVERFLOW_SET(ctx, false);
    ZERO_SET(ctx, value == 0);
    NEGATIVE_SET(ctx, BIT(value, 31) == 1);

    return 0;
}

Instruction* gen_moveq(uint16_t opcode)
{
    Instruction* i = instruction_make("MOVEQ", moveq);
    i->size = Long;
    i->src = operand_make_value(BYTE_LO(opcode), i);
    i->dst = operand_make_data_register(FRAGMENT(opcode, 11, 9), i);
    i->base_cycles = cycles_immediate_instruction(i, 4, 0, 0);
    return i;
}

uint8_t movea(Instruction* i, M68k* ctx)
{
    int32_t value = FETCH_EA_AND_GET(i->src, ctx);

    if (i->size == Word)
        value = SIGN_EXTEND_W(value);

    ctx->address_registers[i->dst->n] = value;

    return 0;
}

Instruction* gen_movea(uint16_t opcode)
{
    Instruction* i = instruction_make("MOVEA", movea);
    i->size = operand_size2(FRAGMENT(opcode, 13, 12));
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);
    i->dst = operand_make_address_register(FRAGMENT(opcode, 11, 9), i);
    return i;
}

Instruction* gen_movep(uint16_t opcode)
{
    Instruction* i = instruction_make("MOVEP", not_implemented);
    i->base_cycles = 0; // TODO
    return i;
}

uint8_t move_to_ccr(Instruction* i, M68k* ctx)
{
    // Only update the CCR segment of the status register
    ctx->status = (ctx->status & 0xFFE0) | (FETCH_EA_AND_GET(i->src, ctx) & 0x1F);

    return 0;
}

Instruction* gen_move_to_ccr(uint16_t opcode)
{
    Instruction* i = instruction_make("MOVE to CCR", move_to_ccr);
    i->size = Word;
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);
    i->base_cycles = 12;
    return i;
}

uint8_t pea(Instruction* i, M68k* ctx)
{
    ctx->address_registers[7] -= 4;
    m68k_write_l(ctx, ctx->address_registers[7], FETCH_EA(i->src, ctx));

    return 0;
}

Instruction* gen_pea(uint16_t opcode)
{
    Instruction* i = instruction_make("PEA", pea);
    i->size = Long;
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);
    return i;
}

uint8_t unlk(Instruction* i, M68k* ctx)
{
    // Load the stack pointer with the address register
    ctx->address_registers[7] = ctx->address_registers[i->src->n];

    // Load the address register with the long word at the top of the stack
    ctx->address_registers[i->src->n] = m68k_read_l(ctx, ctx->address_registers[7]);
    ctx->address_registers[7] += 4;

    return 0;
}

Instruction* gen_unlk(uint16_t opcode)
{
    Instruction* i = instruction_make("UNLK", unlk);
    i->size = Long;
    i->src = operand_make_address_register(FRAGMENT(opcode, 2, 0), i);
    i->base_cycles = 12;
    return i;
}
