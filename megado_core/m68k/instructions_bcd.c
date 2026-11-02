#include "instruction.h"
#include "m68k.h"
#include "operands.h"

// TODO all implems should be made static

// https://en.wikipedia.org/wiki/Binary-coded_decimal

static uint8_t packed_bcd_to_binary(uint8_t bcd)
{
    return (bcd & 0xF0) * 10 + (bcd & 0x0F);
}

static uint8_t binary_to_packed_bcd(uint8_t binary)
{
    return (binary / 10) << 4 | (binary % 10);
}

static void gen_xbcd(Instruction* i, uint16_t opcode, char* name, InstructionFunc func)
{
    instruction_init(i, name, func);
    i->size = Byte;

    bool address_register = BIT(opcode, 3);
    if (address_register)
    {
        operand_make_address_register(&i->src, FRAGMENT(opcode, 2, 0), i);
        operand_make_address_register(&i->dst, FRAGMENT(opcode, 11, 9), i);
    }
    else
    {
        operand_make_data_register(&i->src, FRAGMENT(opcode, 2, 0), i);
        operand_make_data_register(&i->dst, FRAGMENT(opcode, 11, 9), i);
    }

    // TODO timing
}

static uint8_t abcd(Instruction* i, M68k* ctx)
{
    uint8_t a = packed_bcd_to_binary(FETCH_EA_AND_GET(&i->src, ctx));
    uint8_t b = packed_bcd_to_binary(FETCH_EA_AND_GET(&i->dst, ctx));
    uint8_t e = EXTENDED(ctx);
    uint8_t result = a + b + e;
    SET(&i->dst, ctx, binary_to_packed_bcd(result));

    bool carry = (uint16_t)a + (uint16_t)b + e > 0xFF;
    EXTENDED_SET(ctx, carry);
    CARRY_SET(ctx, carry);

    // Only change the zero flag if the result is non-zero

    if (result != 0)
    {
        ZERO_SET(ctx, false);
    }

    return 0;
}

void gen_abcd(Instruction* i, uint16_t opcode)
{
    gen_xbcd(i, opcode, "ABCD", abcd);
}

static uint8_t sbcd(Instruction* i, M68k* ctx)
{
    uint8_t a = packed_bcd_to_binary(FETCH_EA_AND_GET(&i->src, ctx));
    uint8_t b = packed_bcd_to_binary(FETCH_EA_AND_GET(&i->dst, ctx));
    uint8_t e = EXTENDED(ctx);
    uint8_t result = b - a - e;
    SET(&i->dst, ctx, binary_to_packed_bcd(result));

    bool borrow = (uint16_t)a + e > (uint16_t)b;
    EXTENDED_SET(ctx, borrow);
    CARRY_SET(ctx, borrow);

    // Only change the zero flag if the result is non-zero

    if (result != 0)
    {
        ZERO_SET(ctx, false);
    }

    return 0;
}

void gen_sbcd(Instruction* i, uint16_t opcode)
{
    gen_xbcd(i, opcode, "SBCD", sbcd);
}

static uint8_t nbcd(Instruction* i, M68k* ctx)
{
    uint8_t x = packed_bcd_to_binary(FETCH_EA_AND_GET(&i->dst, ctx));
    uint8_t e = EXTENDED(ctx);
    uint8_t result = 0 - x - e;
    SET(&i->dst, ctx, binary_to_packed_bcd(result));

    bool borrow = (x == 0xFF) && e;
    EXTENDED_SET(ctx, borrow);
    CARRY_SET(ctx, borrow);

    // Only change the zero flag if the result is non-zero

    if (result != 0)
    {
        ZERO_SET(ctx, false);
    }

    return 0;
}

void gen_nbcd(Instruction* i, uint16_t opcode)
{
    instruction_init(i, "NBCD", nbcd);
    i->size = Byte;
    operand_make(&i->dst, FRAGMENT(opcode, 5, 0), i);
    // TODO timing
}
