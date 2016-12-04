#pragma once

#include <minunit.h>

#include <m68k/m68k.h>

M68k* m = NULL;

void setup()
{
    m = m68k_init();
}

void teardown()
{
    m68k_free(m);
}

#define DATA(n, x) m->data_registers[n] = x
#define DATA_CHECK(n, x) mu_assert_int_eq(x, m->data_registers[n])
#define RUN(opcode) m68k_execute(m, parse_bin(opcode))

MU_TEST(test_and_data)
{
    DATA(3, 0x0F); DATA(5, 0xFF);
    RUN("1100 011 0 01 000101"); // AND.w D3, D5
    DATA_CHECK(3, 0xF); DATA_CHECK(5, 0xF);
}

MU_TEST(test_and_data2)
{
    DATA(3, 0xFF); DATA(5, 0x0F);
    RUN("1100 011 1 01 000101"); // AND.w D3, D5 (reversed)
    DATA_CHECK(3, 0xF); DATA_CHECK(5, 0xF);
}

MU_TEST_SUITE(test_suite_instructions_logic)
{
    MU_SUITE_CONFIGURE(&setup, &teardown);

    MU_RUN_TEST(test_and_data);
    MU_RUN_TEST(test_and_data2);
}
