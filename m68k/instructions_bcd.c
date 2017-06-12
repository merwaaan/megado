#include <stdlib.h>

#include "cycles.h"
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

int abcd(Instruction* i)
{
    uint8_t a = packed_bcd_to_binary(FETCH_EA_AND_GET(i->src));
    uint8_t b = packed_bcd_to_binary(FETCH_EA_AND_GET(i->dst));
    uint8_t result = a + b + EXTENDED(i->context);
    SET(i->dst, binary_to_packed_bcd(result));

    bool carry = (uint16_t)a + (uint16_t)b + EXTENDED(i->context) > 0xFF;
    EXTENDED_SET(i->context, carry);
    CARRY_SET(i->context, carry);

    // Only change the zero flag if the result is nonzero
    if (result != 0)
        ZERO_SET(i->context, false);

    return 0;
}

Instruction* gen_abcd(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "ABCD", abcd);
    i->size = Byte;

    bool address_register = BIT(opcode, 3);
    if (address_register)
    {
        i->src = operand_make_address_register(FRAGMENT(opcode, 2, 0), i);
        i->dst = operand_make_address_register(FRAGMENT(opcode, 11, 9), i);
    }
    else
    {
        i->src = operand_make_data_register(FRAGMENT(opcode, 2, 0), i);
        i->dst = operand_make_data_register(FRAGMENT(opcode, 11, 9), i);
    }

    //TODO timing
    return i;
}

Instruction* gen_nbcd(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "NBCD", not_implemented);
    return i;
}

Instruction* gen_sbcd(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "SBCD", not_implemented);
    return i;
}
