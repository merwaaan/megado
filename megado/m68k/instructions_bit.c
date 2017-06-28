#include <stdbool.h>
#include <stdlib.h>

#include "bit_utils.h"
#include "cycles.h"
#include "instruction.h"
#include "m68k.h"
#include "operands.h"

// TOFO merge versions

Instruction* gen_bit_instruction(uint16_t opcode, M68k* m, char* name, InstructionFunc func, bool immediate)
{
    Instruction* i = instruction_make(m, name, func);

    i->dst = operand_make(FRAGMENT(opcode, 5, 0), i);

    if (i->dst != NULL)
        i->size = i->dst->type == DataRegister ? Long : Byte;

    if (immediate)
        i->src = operand_make_immediate_value(Word, i);
    else
        i->src = operand_make_data_register(FRAGMENT(opcode, 11, 9), i);

    return i;
}

int bchg(Instruction* i)
{
    uint8_t bit = FETCH_EA_AND_GET(i->src) % i->size;

    uint32_t initial = FETCH_EA_AND_GET(i->dst);
    SET(i->dst, BIT_CHG(initial, bit, !BIT(initial, bit)));

    ZERO_SET(i->context, BIT(initial, bit) == 0);

    return 0;
}

Instruction* gen_bchg(uint16_t opcode, M68k* m)
{
    Instruction* i = gen_bit_instruction(opcode, m, "BCHG", bchg, false);

    i->base_cycles = i->size == Long ?
        cycles_bit_manipulation_instruction(i, 8, 0) :
        cycles_bit_manipulation_instruction(i, 0, 8);

    return i;
}

Instruction* gen_bchg_imm(uint16_t opcode, M68k* m)
{
    Instruction* i = gen_bit_instruction(opcode, m, "BCHG", bchg, true);

    i->base_cycles = i->size == Long ?
        cycles_bit_manipulation_instruction(i, 12, 0) :
        cycles_bit_manipulation_instruction(i, 0, 12);

    return i;
}

int bclr(Instruction* i)
{
    uint8_t bit = FETCH_EA_AND_GET(i->src) % i->size;

    int initial = FETCH_EA_AND_GET(i->dst);
    SET(i->dst, BIT_CLR(initial, bit));

    ZERO_SET(i->context, BIT(initial, bit) == 0);

    return 0;
}

Instruction* gen_bclr(uint16_t opcode, M68k* m)
{
    Instruction* i = gen_bit_instruction(opcode, m, "BCLR", bclr, false);

    i->base_cycles = i->size == Long ?
        cycles_bit_manipulation_instruction(i, 10, 0) :
        cycles_bit_manipulation_instruction(i, 0, 8);

    return i;
}

Instruction* gen_bclr_imm(uint16_t opcode, M68k* m)
{
    Instruction* i = gen_bit_instruction(opcode, m, "BCLR", bclr, true);

    i->base_cycles = i->size == Long ?
        cycles_bit_manipulation_instruction(i, 14, 0) :
        cycles_bit_manipulation_instruction(i, 0, 12);

    return i;
}

int bset(Instruction* i)
{
    uint8_t bit = FETCH_EA_AND_GET(i->src) % i->size;

    uint32_t initial = FETCH_EA_AND_GET(i->dst);
    SET(i->dst, BIT_SET(initial, bit));

    ZERO_SET(i->context, BIT(initial, bit) == 0);

    return 0;
}

Instruction* gen_bset(uint16_t opcode, M68k* m)
{
    Instruction* i = gen_bit_instruction(opcode, m, "BSET", bset, false);

    i->base_cycles = i->size == Long ?
        cycles_bit_manipulation_instruction(i, 8, 0) :
        cycles_bit_manipulation_instruction(i, 0, 8);

    return i;
}

Instruction* gen_bset_imm(uint16_t opcode, M68k* m)
{
    Instruction* i = gen_bit_instruction(opcode, m, "BSET", bset, true);

    i->base_cycles = i->size == Long ?
        cycles_bit_manipulation_instruction(i, 12, 0) :
        cycles_bit_manipulation_instruction(i, 0, 12);

    return i;
}

int btst(Instruction* i) // TODO check doc
{
    int bit = FETCH_EA_AND_GET(i->src) % i->size;
    int set = BIT(FETCH_EA_AND_GET(i->dst), bit);

    ZERO_SET(i->context, !set);

    return 0;
}

Instruction* gen_btst(uint16_t opcode, M68k* m)
{
    Instruction* i = gen_bit_instruction(opcode, m, "BTST", btst, false);

    i->base_cycles = i->size == Long ?
        cycles_bit_manipulation_instruction(i, 6, 0) :
        cycles_bit_manipulation_instruction(i, 0, 4);

    return i;
}

Instruction* gen_btst_imm(uint16_t opcode, M68k* m)
{
    Instruction* i = gen_bit_instruction(opcode, m, "BTST", btst, true);

    i->base_cycles = i->size == Long ?
        cycles_bit_manipulation_instruction(i, 10, 0) :
        cycles_bit_manipulation_instruction(i, 0, 8);

    return i;
}
