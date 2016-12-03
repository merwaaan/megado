#pragma once

#include "minunit.h"

#include "../m68k.h"

M68k* m = NULL;

void setup()
{
    m = m68k_init();
}

void teardown()
{
    m68k_free(m);
}

MU_TEST(test_data_register)
{
    m->data_registers[3] = 0xFF;
    m->data_registers[5] = 0x0F;
    m68k_execute(m, parse_bin("1100 011 0 01 000101")); // AND.w D3,D5
    mu_assert_int_eq(1, 1);
}

MU_TEST_SUITE(test_suite_addressing_modes)
{
	MU_SUITE_CONFIGURE(&setup, &teardown);

	MU_RUN_TEST(test_data_register);
}
