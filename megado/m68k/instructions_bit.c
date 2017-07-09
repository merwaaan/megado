#include <stdbool.h>
#include <stdlib.h>

#include "bit_utils.h"
#include "cycles.h"
#include "instruction.h"
#include "m68k.h"
#include "operands.h"

// TOFO merge versions

Instruction* gen_bit_instruction(uint16_t opcode, char* name, InstructionFunc func, bool immediate)
{
    Instruction* i = instruction_make(name, func);

    i->dst = operand_make(FRAGMENT(opcode, 5, 0), i);
    i->size = i->dst->type == DataRegister ? Long : Byte;

    if (immediate)
        i->src = operand_make_immediate_value(Word, i);
    else
        i->src = operand_make_data_register(FRAGMENT(opcode, 11, 9), i);

    return i;
}

uint8_t bchg(Instruction* i, M68k* ctx)
{
    uint8_t bit = FETCH_EA_AND_GET(i->src, ctx) % i->size;

    uint32_t initial = FETCH_EA_AND_GET(i->dst, ctx);
    SET(i->dst, ctx, BIT_CHG(initial, bit, !BIT(initial, bit)));

    ZERO_SET(ctx, BIT(initial, bit) == 0);

    return 0;
}

Instruction* gen_bchg(uint16_t opcode)
{
    Instruction* i = gen_bit_instruction(opcode, "BCHG", bchg, false);

    i->base_cycles = i->size == Long ?
        cycles_bit_manipulation_instruction(i, 8, 0) :
        cycles_bit_manipulation_instruction(i, 0, 8);

    return i;
}

Instruction* gen_bchg_imm(uint16_t opcode)
{
    Instruction* i = gen_bit_instruction(opcode, "BCHG", bchg, true);

    i->base_cycles = i->size == Long ?
        cycles_bit_manipulation_instruction(i, 12, 0) :
        cycles_bit_manipulation_instruction(i, 0, 12);

    return i;
}

uint8_t bclr(Instruction* i, M68k* ctx)
{
    uint8_t bit = FETCH_EA_AND_GET(i->src, ctx) % i->size;

    int initial = FETCH_EA_AND_GET(i->dst, ctx);
    SET(i->dst, ctx, BIT_CLR(initial, bit));

    ZERO_SET(ctx, BIT(initial, bit) == 0);

    return 0;
}

Instruction* gen_bclr(uint16_t opcode)
{
    Instruction* i = gen_bit_instruction(opcode, "BCLR", bclr, false);

    i->base_cycles = i->size == Long ?
        cycles_bit_manipulation_instruction(i, 10, 0) :
        cycles_bit_manipulation_instruction(i, 0, 8);

    return i;
}

Instruction* gen_bclr_imm(uint16_t opcode)
{
    Instruction* i = gen_bit_instruction(opcode, "BCLR", bclr, true);

    i->base_cycles = i->size == Long ?
        cycles_bit_manipulation_instruction(i, 14, 0) :
        cycles_bit_manipulation_instruction(i, 0, 12);

    return i;
}

uint8_t bset(Instruction* i, M68k* ctx)
{
    uint8_t bit = FETCH_EA_AND_GET(i->src, ctx) % i->size;

    uint32_t initial = FETCH_EA_AND_GET(i->dst, ctx);
    SET(i->dst, ctx, BIT_SET(initial, bit));

    ZERO_SET(ctx, BIT(initial, bit) == 0);

    return 0;
}

Instruction* gen_bset(uint16_t opcode)
{
    Instruction* i = gen_bit_instruction(opcode, "BSET", bset, false);

    i->base_cycles = i->size == Long ?
        cycles_bit_manipulation_instruction(i, 8, 0) :
        cycles_bit_manipulation_instruction(i, 0, 8);

    return i;
}

Instruction* gen_bset_imm(uint16_t opcode)
{
    Instruction* i = gen_bit_instruction(opcode, "BSET", bset, true);

    i->base_cycles = i->size == Long ?
        cycles_bit_manipulation_instruction(i, 12, 0) :
        cycles_bit_manipulation_instruction(i, 0, 12);

    return i;
}

uint8_t btst(Instruction* i, M68k* ctx) // TODO check doc
{
    int bit = FETCH_EA_AND_GET(i->src, ctx) % i->size;
    int set = BIT(FETCH_EA_AND_GET(i->dst, ctx), bit);

    ZERO_SET(ctx, set == 0);

    return 0;
}

Instruction* gen_btst(uint16_t opcode)
{
    Instruction* i = gen_bit_instruction(opcode, "BTST", btst, false);

    i->base_cycles = i->size == Long ?
        cycles_bit_manipulation_instruction(i, 6, 0) :
        cycles_bit_manipulation_instruction(i, 0, 4);

    return i;
}

Instruction* gen_btst_imm(uint16_t opcode)
{
    Instruction* i = gen_bit_instruction(opcode, "BTST", btst, true);

    i->base_cycles = i->size == Long ?
        cycles_bit_manipulation_instruction(i, 10, 0) :
        cycles_bit_manipulation_instruction(i, 0, 8);

    return i;
}
