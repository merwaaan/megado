#include "cycles.h"
#include "instruction.h"
#include "m68k.h"
#include "operands.h"

static void gen_xbcd(Instruction* i, uint16_t opcode, char* name, InstructionFunc func)
{
    instruction_init(i, name, func);
    i->size = Byte;

    bool address_register = BIT(opcode, 3);
    if (address_register)
    {
        operand_make_address_register_indirect_predec(&i->src, FRAGMENT(opcode, 2, 0), i);
        operand_make_address_register_indirect_predec(&i->dst, FRAGMENT(opcode, 11, 9), i);
    }
    else
    {
        operand_make_data_register(&i->src, FRAGMENT(opcode, 2, 0), i);
        operand_make_data_register(&i->dst, FRAGMENT(opcode, 11, 9), i);
    }

    i->base_cycles = address_register ? 18 : 6;
}

static uint8_t abcd(Instruction* i, M68k* ctx)
{
    uint8_t a = FETCH_EA_AND_GET(&i->src, ctx);
    uint8_t b = FETCH_EA_AND_GET(&i->dst, ctx);
    uint8_t e = EXTENDED(ctx);

    uint16_t result = a + b + e;

    // Apply BCD correction

    if ((a & 0x0F) + (b & 0x0F) + e > 9)
    {
        result += 0x06;
    }

    uint8_t carry = result > 0x99;

    if (carry)
    {
        result += 0x60;
    }

    SET(&i->dst, ctx, result);

    EXTENDED_SET(ctx, carry);
    CARRY_SET(ctx, carry);

    // Only change the zero flag if the result is non-zero

    if ((result & 0xFF) != 0)
    {
        ZERO_SET(ctx, false);
    }

    return 0;
}

void gen_abcd(Instruction* i, uint16_t opcode)
{
    gen_xbcd(i, opcode, "ABCD", abcd);
}

// Common logic for SBCD and NBCD
static uint8_t subtract_bcd(M68k* ctx, uint8_t dst, uint8_t src)
{
    uint8_t e = EXTENDED(ctx);

    uint16_t result = dst - src - e;

    // Apply BCD correction

    if ((dst & 0x0F) < (src & 0x0F) + e)
    {
        result -= 0x06;
    }

    uint8_t borrow = result > 0x99;

    if (borrow)
    {
        result -= 0x60;
    }

    EXTENDED_SET(ctx, borrow);
    CARRY_SET(ctx, borrow);

    // Only change the zero flag if the result is non-zero

    if ((result & 0xFF) != 0)
    {
        ZERO_SET(ctx, false);
    }

    return result;
}

static uint8_t sbcd(Instruction* i, M68k* ctx)
{
    uint8_t a = FETCH_EA_AND_GET(&i->src, ctx);
    uint8_t b = FETCH_EA_AND_GET(&i->dst, ctx);

    SET(&i->dst, ctx, subtract_bcd(ctx, b, a));

    return 0;
}

void gen_sbcd(Instruction* i, uint16_t opcode)
{
    gen_xbcd(i, opcode, "SBCD", sbcd);
}

static uint8_t nbcd(Instruction* i, M68k* ctx)
{
    // NBCD is equivalent to SBCD with a zero source

    uint8_t a = FETCH_EA_AND_GET(&i->dst, ctx);

    SET(&i->dst, ctx, subtract_bcd(ctx, 0, a));

    return 0;
}

void gen_nbcd(Instruction* i, uint16_t opcode)
{
    instruction_init(i, "NBCD", nbcd);
    i->size = Byte;

    operand_make(&i->dst, FRAGMENT(opcode, 5, 0), i);

    i->base_cycles = cycles_single_operand_instruction(i, 6, 8);
}
