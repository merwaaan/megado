#include <stdbool.h>

#include "bit_utils.h"
#include "cycles.h"
#include "instruction.h"
#include "m68k.h"
#include "operands.h"

// TOFO merge versions

void gen_bit_instruction(Instruction* i, uint16_t opcode, char* name, InstructionFunc func, bool immediate)
{
    instruction_init(i, name, func);

    operand_make(&i->dst, FRAGMENT(opcode, 5, 0), i);
    i->size = i->dst.type == DataRegister ? Long : Byte;

    if (immediate)
    {
        operand_make_immediate_value(&i->src, Word, i);
    }
    else
    {
        operand_make_data_register(&i->src, FRAGMENT(opcode, 11, 9), i);
    }
}

uint8_t bchg(Instruction* i, M68k* ctx)
{
    uint8_t bit = FETCH_EA_AND_GET(&i->src, ctx) % i->size;

    uint32_t initial = FETCH_EA_AND_GET(&i->dst, ctx);
    SET(&i->dst, ctx, BIT_CHG(initial, bit, !BIT(initial, bit)));

    ZERO_SET(ctx, BIT(initial, bit) == 0);

    return 0;
}

void gen_bchg(Instruction* i, uint16_t opcode)
{
    gen_bit_instruction(i, opcode, "BCHG", bchg, false);

    i->base_cycles = i->size == Long ? cycles_bit_manipulation_instruction(i, 8, 0) : cycles_bit_manipulation_instruction(i, 0, 8);
}

void gen_bchg_imm(Instruction* i, uint16_t opcode)
{
    gen_bit_instruction(i, opcode, "BCHG", bchg, true);

    i->base_cycles = i->size == Long ? cycles_bit_manipulation_instruction(i, 12, 0) : cycles_bit_manipulation_instruction(i, 0, 12);
}

uint8_t bclr(Instruction* i, M68k* ctx)
{
    uint8_t bit = FETCH_EA_AND_GET(&i->src, ctx) % i->size;

    int initial = FETCH_EA_AND_GET(&i->dst, ctx);
    SET(&i->dst, ctx, BIT_CLR(initial, bit));

    ZERO_SET(ctx, BIT(initial, bit) == 0);

    return 0;
}

void gen_bclr(Instruction* i, uint16_t opcode)
{
    gen_bit_instruction(i, opcode, "BCLR", bclr, false);

    i->base_cycles = i->size == Long ? cycles_bit_manipulation_instruction(i, 10, 0) : cycles_bit_manipulation_instruction(i, 0, 8);
}

void gen_bclr_imm(Instruction* i, uint16_t opcode)
{
    gen_bit_instruction(i, opcode, "BCLR", bclr, true);

    i->base_cycles = i->size == Long ? cycles_bit_manipulation_instruction(i, 14, 0) : cycles_bit_manipulation_instruction(i, 0, 12);
}

uint8_t bset(Instruction* i, M68k* ctx)
{
    uint8_t bit = FETCH_EA_AND_GET(&i->src, ctx) % i->size;

    uint32_t initial = FETCH_EA_AND_GET(&i->dst, ctx);
    SET(&i->dst, ctx, BIT_SET(initial, bit));

    ZERO_SET(ctx, BIT(initial, bit) == 0);

    return 0;
}

void gen_bset(Instruction* i, uint16_t opcode)
{
    gen_bit_instruction(i, opcode, "BSET", bset, false);

    i->base_cycles = i->size == Long ? cycles_bit_manipulation_instruction(i, 8, 0) : cycles_bit_manipulation_instruction(i, 0, 8);
}

void gen_bset_imm(Instruction* i, uint16_t opcode)
{
    gen_bit_instruction(i, opcode, "BSET", bset, true);

    i->base_cycles = i->size == Long ? cycles_bit_manipulation_instruction(i, 12, 0) : cycles_bit_manipulation_instruction(i, 0, 12);
}

uint8_t btst(Instruction* i, M68k* ctx) // TODO check doc
{
    int bit = FETCH_EA_AND_GET(&i->src, ctx) % i->size;
    int set = BIT(FETCH_EA_AND_GET(&i->dst, ctx), bit);

    ZERO_SET(ctx, set == 0);

    return 0;
}

void gen_btst(Instruction* i, uint16_t opcode)
{
    gen_bit_instruction(i, opcode, "BTST", btst, false);

    i->base_cycles = i->size == Long ? cycles_bit_manipulation_instruction(i, 6, 0) : cycles_bit_manipulation_instruction(i, 0, 4);
}

void gen_btst_imm(Instruction* i, uint16_t opcode)
{
    gen_bit_instruction(i, opcode, "BTST", btst, true);

    i->base_cycles = i->size == Long ? cycles_bit_manipulation_instruction(i, 10, 0) : cycles_bit_manipulation_instruction(i, 0, 8);
}
