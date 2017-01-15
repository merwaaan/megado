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

MU_TEST(test_data_b)
{
    Operand* o = operand_make_data_register(1, instr);
    o->instruction->size = Byte;

    DATA(1, 0xFF);
    ASSERT(0xFF, GET(o));

    SET(o, 0x1F);
    DATA_CHECK(1, 0x1F);

    DATA(1, 0xAAFF);
    ASSERT(0xFF, GET(o));

    SET(o, 0x1F1F);
    DATA_CHECK(1, 0xAA1F);
}

MU_TEST(test_data_w)
{
    Operand* o = operand_make_data_register(1, instr);
    o->instruction->size = Word;

    DATA(1, 0xFFFF);
    ASSERT(0xFFFF, GET(o));

    SET(o, 0x1F1F);
    DATA_CHECK(1, 0x1F1F);

    DATA(1, 0xAAAAFFFF);
    ASSERT(0xFFFF, GET(o));

    SET(o, 0xBBBB1F1F);
    DATA_CHECK(1, 0xAAAA1F1F);
}

MU_TEST(test_data_l)
{
    Operand* o = operand_make_data_register(1, instr);
    o->instruction->size = Long;

    DATA(1, 0xFFFFFFFF);
    ASSERT(0xFFFFFFFF, GET(o));

    SET(o, 0x1F1FFFFF);
    DATA_CHECK(1, 0x1F1FFFFF);
}

MU_TEST(test_address_b)
{
    Operand* o = operand_make_address_register(3, instr);
    o->instruction->size = Byte;

    ADDR(3, 0xAA);
    ASSERT(0xAA, GET(o));

    SET(o, 0xEE);
    ADDR_CHECK(3, 0xEE);

    ADDR(3, 0xAAAA);
    ASSERT(0xAA, GET(o));

    SET(o, 0xBBBB);
    ADDR_CHECK(3, 0xAABB);
}

MU_TEST(test_address_w)
{
    Operand* o = operand_make_address_register(3, instr);
    o->instruction->size = Word;

    ADDR(3, 0xAAAA);
    ASSERT(0xAAAA, GET(o));

    SET(o, 0xEEEE);
    ADDR_CHECK(3, 0xEEEE);

    ADDR(3, 0xAAAAAAAA);
    ASSERT(0xAAAA, GET(o));

    SET(o, 0xBBBBBBBB);
    ADDR_CHECK(3, 0xAAAABBBB);
}

MU_TEST(test_address_l)
{
    Operand* o = operand_make_address_register(3, instr);
    o->instruction->size = Long;

    ADDR(3, 0xAAAAAAAA);
    ASSERT(0xAAAAAAAA, GET(o));

    SET(o, 0xEEEEFFFF);
    ADDR_CHECK(3, 0xEEEEFFFF);
}

MU_TEST(test_address_indirect_b)
{
    Operand* o = operand_make_address_register_indirect(5, instr);
    o->instruction->size = Byte;

    ADDR(5, 0x123ABC);
    MEM(0x123ABC, 0xEF);
    ASSERT(0xEF, GET(o));

    SET(o, 0xCC);
    MEM_CHECK(0x123ABC, 0xCC);

    MEM_W(0x123ABC, 0xABCD);
    ASSERT(0xAB, GET(o));

    SET(o, 0x1234);
    MEM_CHECK_W(0x123ABC, 0x34CD);
}

MU_TEST(test_address_indirect_w)
{
    Operand* o = operand_make_address_register_indirect(5, instr);
    o->instruction->size = Word;

    ADDR(5, 0x123ABC);
    MEM_W(0x123ABC, 0xEFEF);
    ASSERT(0xEFEF, GET(o));

    SET(o, 0xCCCC);
    MEM_CHECK_W(0x123ABC, 0xCCCC);

    MEM_L(0x123ABC, 0x12345678);
    ASSERT(0x1234, GET(o));

    SET(o, 0xCCCC);
    MEM_CHECK_L(0x123ABC, 0xCCCC5678);
}

MU_TEST(test_address_indirect_l)
{
    Operand* o = operand_make_address_register_indirect(5, instr);
    o->instruction->size = Long;

    ADDR(5, 0x123ABC);
    MEM_L(0x123ABC, 0xEFEF0000);
    ASSERT(0xEFEF0000, GET(o));

    SET(o, 0xCCCC0000);
    MEM_CHECK_L(0x123ABC, 0xCCCC0000);
}

MU_TEST_SUITE(test_suite_addressing_modes)
{
    MU_SUITE_CONFIGURE(&setup_addressing_modes, &teardown_addressing_modes);

    MU_RUN_TEST(test_data_b);
    MU_RUN_TEST(test_data_w);
    MU_RUN_TEST(test_data_l);

    MU_RUN_TEST(test_address_b);
    MU_RUN_TEST(test_address_w);
    MU_RUN_TEST(test_address_l);

    MU_RUN_TEST(test_address_indirect_b);
    MU_RUN_TEST(test_address_indirect_w);
    MU_RUN_TEST(test_address_indirect_l);

    // TODO ...
}
