#include <greatest.h>
#include <m68k/instruction.h>
#include <m68k/m68k.h>
#include <m68k/operands.h>

#include "globals.h"

// Test operand instantiated differently for each test
static Operand o;

// Mock instruction to bind the test operand to the M68K context
static Instruction instr;

static void setup_addressing_modes(void* data)
{
    setup(data);
    instruction_init(&instr, "MOCK", NULL);
}

static void teardown_addressing_modes(void* data)
{
    teardown(data);
}

TEST test_data_b()
{
    operand_make_data_register(&o, 1, &instr);
    o.instruction->size = Byte;

    DATA(1, 0xFF);
    CHECK(0xFF, FETCH_EA_AND_GET(&o, m));

    FETCH_EA_AND_SET(&o, m, 0x1F);
    DATA_CHECK(1, 0x1F);

    DATA(1, 0xAAFF);
    CHECK(0xFF, FETCH_EA_AND_GET(&o, m));

    FETCH_EA_AND_SET(&o, m, 0x1F1F);
    DATA_CHECK(1, 0xAA1F);
    PASS();
}

TEST test_data_w()
{
    operand_make_data_register(&o, 1, &instr);
    o.instruction->size = Word;

    DATA(1, 0xFFFF);
    CHECK(0xFFFF, FETCH_EA_AND_GET(&o, m));

    FETCH_EA_AND_SET(&o, m, 0x1F1F);
    DATA_CHECK(1, 0x1F1F);

    DATA(1, 0xAAAAFFFF);
    CHECK(0xFFFF, FETCH_EA_AND_GET(&o, m));

    FETCH_EA_AND_SET(&o, m, 0xBBBB1F1F);
    DATA_CHECK(1, 0xAAAA1F1F);
    PASS();
}

TEST test_data_l()
{
    operand_make_data_register(&o, 1, &instr);
    o.instruction->size = Long;

    DATA(1, 0xFFFFFFFF);
    CHECK(0xFFFFFFFF, FETCH_EA_AND_GET(&o, m));

    FETCH_EA_AND_SET(&o, m, 0x1F1FFFFF);
    DATA_CHECK(1, 0x1F1FFFFF);
    PASS();
}

TEST test_address_b()
{
    operand_make_address_register(&o, 3, &instr);
    o.instruction->size = Byte;

    ADDR(3, 0xAA);
    CHECK(0xAA, FETCH_EA_AND_GET(&o, m));

    FETCH_EA_AND_SET(&o, m, 0xEE);
    ADDR_CHECK(3, 0xEE);

    ADDR(3, 0xAAAA);
    CHECK(0xAA, FETCH_EA_AND_GET(&o, m));

    FETCH_EA_AND_SET(&o, m, 0xBBBB);
    ADDR_CHECK(3, 0xAABB);
    PASS();
}

TEST test_address_w()
{
    operand_make_address_register(&o, 3, &instr);
    o.instruction->size = Word;

    ADDR(3, 0xAAAA);
    CHECK(0xAAAA, FETCH_EA_AND_GET(&o, m));

    FETCH_EA_AND_SET(&o, m, 0xEEEE);
    ADDR_CHECK(3, 0xEEEE);

    ADDR(3, 0xAAAAAAAA);
    CHECK(0xAAAA, FETCH_EA_AND_GET(&o, m));

    FETCH_EA_AND_SET(&o, m, 0xBBBBBBBB);
    ADDR_CHECK(3, 0xAAAABBBB);
    PASS();
}

TEST test_address_l()
{
    operand_make_address_register(&o, 3, &instr);
    o.instruction->size = Long;

    ADDR(3, 0xAAAAAAAA);
    CHECK(0xAAAAAAAA, FETCH_EA_AND_GET(&o, m));

    FETCH_EA_AND_SET(&o, m, 0xEEEEFFFF);
    ADDR_CHECK(3, 0xEEEEFFFF);
    PASS();
}

TEST test_address_indirect_b()
{
    operand_make_address_register_indirect(&o, 5, &instr);
    o.instruction->size = Byte;

    ADDR(5, 0x123ABC);
    MEM(0x123ABC, 0xEF);
    CHECK(0xEF, FETCH_EA_AND_GET(&o, m));

    FETCH_EA_AND_SET(&o, m, 0xCC);
    MEM_CHECK(0x123ABC, 0xCC);

    MEM_W(0x123ABC, 0xABCD);
    CHECK(0xAB, FETCH_EA_AND_GET(&o, m));

    FETCH_EA_AND_SET(&o, m, 0x1234);
    MEM_CHECK_W(0x123ABC, 0x34CD);
    PASS();
}

TEST test_address_indirect_w()
{
    operand_make_address_register_indirect(&o, 5, &instr);
    o.instruction->size = Word;

    ADDR(5, 0x123ABC);
    MEM_W(0x123ABC, 0xEFEF);
    CHECK(0xEFEF, FETCH_EA_AND_GET(&o, m));

    FETCH_EA_AND_SET(&o, m, 0xCCCC);
    MEM_CHECK_W(0x123ABC, 0xCCCC);

    MEM_L(0x123ABC, 0x12345678);
    CHECK(0x1234, FETCH_EA_AND_GET(&o, m));

    FETCH_EA_AND_SET(&o, m, 0xCCCC);
    MEM_CHECK_L(0x123ABC, 0xCCCC5678);
    PASS();
}

TEST test_address_indirect_l()
{
    operand_make_address_register_indirect(&o, 5, &instr);
    o.instruction->size = Long;

    ADDR(5, 0x123ABC);
    MEM_L(0x123ABC, 0xEFEF0000);
    CHECK(0xEFEF0000, FETCH_EA_AND_GET(&o, m));

    FETCH_EA_AND_SET(&o, m, 0xCCCC0000);
    MEM_CHECK_L(0x123ABC, 0xCCCC0000);
    PASS();
}

SUITE(test_suite_addressing_modes)
{
    SET_SETUP(setup_addressing_modes, NULL);
    SET_TEARDOWN(teardown_addressing_modes, NULL);

    RUN_TEST(test_data_b);
    RUN_TEST(test_data_w);
    RUN_TEST(test_data_l);

    RUN_TEST(test_address_b);
    RUN_TEST(test_address_w);
    RUN_TEST(test_address_l);

    RUN_TEST(test_address_indirect_b);
    RUN_TEST(test_address_indirect_w);
    RUN_TEST(test_address_indirect_l);

    // TODO ...
}
