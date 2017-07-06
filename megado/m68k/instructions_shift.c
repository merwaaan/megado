#include <stdbool.h>
#include <stdlib.h>

#include "bit_utils.h"
#include "cycles.h"
#include "instruction.h"
#include "m68k.h"
#include "operands.h"

// TODO check all timings

Instruction* gen_shift_instruction(uint16_t opcode, char* name, InstructionFunc func)
{
    Instruction* i = instruction_make(name, func);
    i->size = operand_size(FRAGMENT(opcode, 7, 6));
    i->dst = operand_make_data_register(FRAGMENT(opcode, 2, 0), i);

    bool reg = BIT(opcode, 5);
    if (reg)
        i->src = operand_make_data_register(FRAGMENT(opcode, 11, 9), i);
    else
        i->src = operand_make_value(FRAGMENT(opcode, 11, 9), i);

    i->base_cycles = i->size == Long ? 8 : 6;

    return i;
}

Instruction* gen_shift_memory_instruction(uint16_t opcode, char* name, InstructionFunc func)
{
    Instruction* i = instruction_make(name, func);
    i->size = Word;
    i->src = operand_make_value(1, i); // The shift count is always 1
    i->dst = operand_make(FRAGMENT(opcode, 5, 0), i);
    i->base_cycles = 8 + lookup_cycles_ea(i->size, i->dst->type);
    return i;
}

uint8_t asr(Instruction* i, M68k* ctx)
{
    uint32_t initial = FETCH_EA_AND_GET(i->dst, ctx);

    uint8_t shift = FETCH_EA_AND_GET(i->src, ctx);
    if (shift == 0)
        shift = 8;

    if (shift > 0)
    {
        uint32_t shifted_in = BIT(initial, i->size - 1) ? MASK_BELOW(0xFFFFFFFF, i->size - shift) : 0;
        SET(i->dst, ctx, initial >> shift | shifted_in);
        
        uint8_t last_shifted_out = shift > i->size ? 0 : BIT(initial, shift - 1);
        CARRY_SET(ctx, last_shifted_out);
        EXTENDED_SET(ctx, last_shifted_out);
    }

    uint32_t result = GET(i->dst, ctx);
    OVERFLOW_SET(ctx, false);
    ZERO_SET(ctx, result == 0);
    NEGATIVE_SET(ctx, BIT(result, i->size - 1) == 1);

    return 2 * shift;
}

uint8_t lsl(Instruction* i, M68k* ctx)
{
    uint32_t initial = FETCH_EA_AND_GET(i->dst, ctx);

    uint8_t shift = FETCH_EA_AND_GET(i->src, ctx);
    if (shift == 0)
        shift = 8;

    if (shift > 0) // TODO always > 0!!
    {
        SET(i->dst, ctx, initial << shift);

        uint8_t last_shifted_out = shift > i->size ? 0 : BIT(initial, i->size - shift);
        EXTENDED_SET(ctx, last_shifted_out);
        CARRY_SET(ctx, last_shifted_out);
    }

    uint32_t result = GET(i->dst, ctx);
    OVERFLOW_SET(ctx, false);
    ZERO_SET(ctx, result == 0);
    NEGATIVE_SET(ctx, BIT(result, i->size - 1) == 1);

    return 2 * shift;
}

uint8_t lsr(Instruction* i, M68k* ctx)
{
    uint32_t initial = FETCH_EA_AND_GET(i->dst, ctx);

    uint8_t shift = FETCH_EA_AND_GET(i->src, ctx);
    if (shift == 0)
        shift = 8;

    if (shift > 0) // TODO always > 0!!
    {
        SET(i->dst, ctx, initial >> shift);

        uint8_t last_shifted_out = shift > i->size ? 0 : BIT(initial, shift - 1);
        CARRY_SET(ctx, last_shifted_out);
        EXTENDED_SET(ctx, last_shifted_out);
    }

    uint32_t result = GET(i->dst, ctx);
    OVERFLOW_SET(ctx, false);
    ZERO_SET(ctx, result == 0);
    NEGATIVE_SET(ctx, BIT(result, i->size - 1) == 1);

    return 2 * shift;
}

Instruction* gen_asd(uint16_t opcode)
{
    bool direction = BIT(opcode, 8);
    return gen_shift_instruction(opcode, direction ? "ASL" : "ASR", direction ? lsl : asr); // ASL = LSL
}

Instruction* gen_asd_mem(uint16_t opcode)
{
    bool direction = BIT(opcode, 8);
    return gen_shift_memory_instruction(opcode, direction ? "ASL" : "ASR", direction ? lsl : asr); // ASL = LSL
}

Instruction* gen_lsd(uint16_t opcode)
{
    bool direction = BIT(opcode, 8);
    return gen_shift_instruction(opcode, direction ? "LSL" : "LSR", direction ? lsl : lsr);
}

Instruction* gen_lsd_mem(uint16_t opcode)
{
    bool direction = BIT(opcode, 8);
    return gen_shift_memory_instruction(opcode, direction ? "LSL" : "LSR", direction ? lsl : lsr);
}

uint8_t rol(Instruction* i, M68k* ctx)
{
    uint32_t initial = FETCH_EA_AND_GET(i->dst, ctx);

    int rotation = FETCH_EA_AND_GET(i->src, ctx) % i->size;
    if (rotation == 0)
        rotation = 8;

    if (rotation > 0) // TODO always true!
    {
        SET(i->dst, ctx, initial << rotation | FRAGMENT(initial, i->size - 1, i->size - rotation));

        CARRY_SET(ctx, BIT(initial, i->size - rotation));
    }
    else
        CARRY_SET(ctx, 0);

    uint32_t result = GET(i->dst, ctx);
    OVERFLOW_SET(ctx, false);
    ZERO_SET(ctx, result == 0);
    NEGATIVE_SET(ctx, BIT(result, i->size - 1) == 1);

    return 2 * rotation;
}

uint8_t ror(Instruction* i, M68k* ctx)
{
    uint32_t initial = FETCH_EA_AND_GET(i->dst, ctx);

    int rotation = FETCH_EA_AND_GET(i->src, ctx) % i->size;
    if (rotation == 0)
        rotation = 8;

    if (rotation > 0) //TODO always true!
    {
        SET(i->dst, ctx, initial >> rotation | FRAGMENT(initial, rotation - 1, 0) << (i->size - rotation));

        CARRY_SET(ctx, BIT(initial, rotation - 1));
    }
    else
        CARRY_SET(ctx, 0);

    uint32_t result = GET(i->dst, ctx);
    OVERFLOW_SET(ctx, false);
    ZERO_SET(ctx, result == 0);
    NEGATIVE_SET(ctx, BIT(result, i->size - 1) == 1);

    return 2 * rotation;
}

Instruction* gen_rod(uint16_t opcode)
{
    bool direction = BIT(opcode, 8);
    return gen_shift_instruction(opcode, direction ? "ROL" : "ROR", direction ? rol : ror);
}

Instruction* gen_rod_mem(uint16_t opcode)
{
    bool direction = BIT(opcode, 8);
    return gen_shift_memory_instruction(opcode, direction ? "ROL" : "ROR", direction ? rol : ror);
}

uint8_t roxl(Instruction* i, M68k* ctx)
{
    uint32_t initial = FETCH_EA_AND_GET(i->dst, ctx);

    int rotation = GET(i->src, ctx) % i->size;
    if (rotation == 0)
        rotation = 8;

    if (rotation > 0) // TODO always true
    {
        uint32_t rotated = rotation > 1 ? FRAGMENT(initial, i->size - 1, i->size - rotation + 1) : 0;
        SET(i->dst, ctx, initial << rotation | EXTENDED(ctx) << (rotation - 1) | rotated);

        uint8_t last_shifted_out = BIT(initial, i->size - rotation);
        CARRY_SET(ctx, last_shifted_out);
        EXTENDED_SET(ctx, last_shifted_out);
    }
    else
        CARRY_SET(ctx, EXTENDED(ctx));

    uint32_t result = GET(i->dst, ctx);
    OVERFLOW_SET(ctx, false);
    ZERO_SET(ctx, result == 0);
    NEGATIVE_SET(ctx, BIT(result, i->size - 1) == 1);

    return 2 * rotation;
}

uint8_t roxr(Instruction* i, M68k* ctx)
{
    FETCH_EA(i->dst, ctx);
    uint32_t initial = GET(i->dst, ctx);

    int rotation = GET(i->src, ctx) % i->size;
    if (rotation > 0)
    {
        SET(i->dst, ctx, initial >> rotation | FRAGMENT(initial, rotation - 1, 0) << (i->size - rotation + 1) | EXTENDED(ctx) << (i->size - rotation));

        uint8_t last_shifted_out = BIT(initial, rotation - 1);
        CARRY_SET(ctx, last_shifted_out);
        EXTENDED_SET(ctx, last_shifted_out);
    }

    uint32_t result = GET(i->dst, ctx);
    OVERFLOW_SET(ctx, false);
    ZERO_SET(ctx, result == 0);
    NEGATIVE_SET(ctx, BIT(result, i->size - 1) == 1);

    return 2 * rotation;
}

Instruction* gen_roxd(uint16_t opcode)
{
    bool direction = BIT(opcode, 8);
    return gen_shift_instruction(opcode, direction ? "ROXL" : "ROXR", direction ? roxl : roxr);
}

Instruction* gen_roxd_mem(uint16_t opcode)
{
    bool direction = BIT(opcode, 8);
    return gen_shift_memory_instruction(opcode, direction ? "ROXL" : "ROXR", direction ? roxl : roxr);
}

uint8_t swap(Instruction* i, M68k* ctx)
{
    uint32_t value = GET(i->src, ctx);
    uint32_t lo = value & 0xFFFF;
    uint16_t hi = value >> 16;
    uint32_t result = (lo << 16) | hi;
    SET(i->src, ctx, result);

    CARRY_SET(ctx, false);
    OVERFLOW_SET(ctx, false);
    ZERO_SET(ctx, result == 0);
    NEGATIVE_SET(ctx, BIT(result, 31));

    return 0;
}

Instruction* gen_swap(uint16_t opcode)
{
    Instruction* i = instruction_make("SWAP", swap);
    i->size = Long;
    i->src = operand_make_data_register(FRAGMENT(opcode, 5, 0), i);
    i->base_cycles = 4;
    return i;
}
