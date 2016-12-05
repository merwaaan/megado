#pragma once

#include <minunit.h>
#include <m68k/m68k.h>

#include "globals.h"

MU_TEST(test_bchg_set)
{
    DATA(1, 2); DATA(5, 0xFFFFFFFF);
    RUN("0000 010 101 000101"); // BCHG D2, D5
    DATA_CHECK(5, 0xFFFFFFFB);
}

MU_TEST(test_bchg_cleared)
{
    DATA(1, 2); DATA(5, 0xFFFFFFFB);
    RUN("0000 010 101 000101"); // BCHG D2, D5
    DATA_CHECK(5, 0xFFFFFFFF);
}

MU_TEST(test_bclr)
{
    DATA(1, 2); DATA(0, 0xFFFFFFFF);
    RUN("0000 001 111 000000"); // BCLR D1, D0
    DATA_CHECK(5, 0xFFFFFFFB);
}

MU_TEST(test_bset)
{
    DATA(0, 2); DATA(5, 0);
    RUN("0000 010 111 000101"); // BSET D0, D5
    DATA_CHECK(5, 4);
}

MU_TEST(test_btst_true)
{
    DATA(6, 5); DATA(7, 0x1A0);
    RUN("0000 110 100 000111"); // BTST D6, D7
    mu_assert_int_eq(ZERO(m), 1);
}

MU_TEST(test_btst_false)
{
    DATA(6, 2); DATA(7, 0x1A0);
    RUN("0000 110 100 000111"); // BTST D6, D7
    mu_assert_int_eq(ZERO(m), 0);
}

MU_TEST_SUITE(test_suite_instructions_bit)
{
    MU_SUITE_CONFIGURE(&setup, &teardown);

    MU_RUN_TEST(test_bchg_set);
    MU_RUN_TEST(test_bchg_cleared);
    MU_RUN_TEST(test_bclr);
    MU_RUN_TEST(test_bset);
    MU_RUN_TEST(test_btst_true);
    MU_RUN_TEST(test_btst_false);
}
