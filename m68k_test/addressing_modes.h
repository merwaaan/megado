#pragma once

#include <minunit.h>
#include <m68k/m68k.h>
#include <m68k/instruction.h>
#include <m68k/operands.h>

#include "globals.h"

// Test operand instantiated differently for each test
Operand* o;

// Mock instruction to bind the test operand to the M68K context
Instruction* instr;

void setup_addressing_modes()
{
    setup();
    instr = instruction_make(m, "MOCK", NULL);
}

void teardown_addressing_modes()
{
    instruction_free(instr);
    teardown();
}


MU_TEST(test_data_register)
{
    return;
    DATA(1, 0xFF);

    Operand* o = operand_make_data_register(1, instr);

    uint32_t value = GET(o);
    mu_assert_int_eq_hex(0xFF, value);

    SET(o, 0x1F);
    DATA_CHECK(1, 0x1F);
}

MU_TEST(test_address_register)
{
    ADDR(3, 0xF0AA);

    Operand* o = operand_make_address_register(3, instr);

    uint32_t value = GET(o);
    mu_assert_int_eq_hex(0xF0AA, value);

    SET(o, 0xEEEE);
    ADDR_CHECK(3, 0xEEEE);
}

MU_TEST(test_address_register_indirect)
{
    ADDR(5, 0x123ABC);
    MEM(0x123ABC, 0xEF);

    Operand* o = operand_make_address_register_indirect(5, instr);

    uint32_t value = GET(o);
    mu_assert_int_eq_hex(0xEF, value);

    SET(o, 0xCC);
    MEM_CHECK(0x123ABC, 0xCC);
}

MU_TEST_SUITE(test_suite_addressing_modes)
{
    MU_SUITE_CONFIGURE(&setup_addressing_modes, &teardown_addressing_modes);

    MU_RUN_TEST(test_data_register);
    MU_RUN_TEST(test_address_register);
    MU_RUN_TEST(test_address_register_indirect);
}
