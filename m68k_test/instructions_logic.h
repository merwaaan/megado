#pragma once

#include <minunit.h>
#include <m68k/m68k.h>
#include <stdbool.h>

#include "globals.h"

MU_TEST(test_and_b)
{
    DATA(2, 0xFE800174);
    DATA(3, 0x25CFB7DD);
    RUN("1100  010 0 00 000011"); // AND.b D2, D3
    DATA_CHECK(3, 0x25CFB754);
}

MU_TEST(test_and_w)
{
    DATA(2, 0xFE800174);
    DATA(3, 0x25CFB7DD);
    RUN("1100  010 0 01 000011"); // AND.w D2, D3
    DATA_CHECK(3, 0x25CF0154);
}

MU_TEST(test_and_l)
{
    DATA(2, 0xFE800174);
    DATA(3, 0x25CFB7DD);
    RUN("1100  010 0 10 000011"); // AND.l D2, D3
    DATA_CHECK(3, 0x24800154);
}

MU_TEST(test_eor_b)
{
    DATA(4, 0x25CFB7DD);
    DATA(1, 0xFE800174);
    RUN("1011 100 0 00 000001"); // EOR.b D4, D1
    DATA_CHECK(1, 0xFE8001A9);
}

MU_TEST(test_eor_w)
{
    DATA(4, 0x25CFB7DD);
    DATA(1, 0xFE800174);
    RUN("1011 100 0 01 000001"); // EOR.w D4, D1
    DATA_CHECK(1, 0xFE80B6A9);
}

MU_TEST(test_eor_l)
{
    DATA(4, 0x25CFB7DD);
    DATA(1, 0xFE800174);
    RUN("1011 100 0 10 000001"); // EOR.l D4, D1
    DATA_CHECK(1, 0xDB4FB6A9);
}

MU_TEST(test_or_b)
{
    DATA(4, 0x25CFB7DD);
    DATA(1, 0xFE800174);
    RUN("1000 100 0 00 000001"); // OR.b D4, D1
    DATA_CHECK(1, 0xFE8001FD);
}

MU_TEST(test_or_w)
{
    DATA(4, 0x25CFB7DD);
    DATA(1, 0xFE800174);
    RUN("1000 100 0 01 000001"); // OR.w D4, D1
    DATA_CHECK(1, 0xFE80B7FD);
}

MU_TEST(test_or_l)
{
    DATA(4, 0x25CFB7DD);
    DATA(1, 0xFE800174);
    RUN("1000 100 0 10 000001"); // OR.l D4, D1
    DATA_CHECK(1, 0xFFCFB7FD);
}

MU_TEST(test_andi_b) // TODO
{
    DATA(0, 0xF0F0F0F0);
    //MEM()
    RUN("00000010 00 000000"); // ANDI.b #FF D2
    DATA_CHECK(0, 0); // TODO
}

// TODO ori, eori

MU_TEST(test_not_b)
{
    DATA(7, 0xFE800174);
    RUN("01000110 00 000111"); // NOT.b D7
    DATA_CHECK(7, 0xFE80018B);
}

MU_TEST(test_not_w)
{
    DATA(7, 0xFE800174);
    RUN("01000110 01 000111"); // NOT.w D7
    DATA_CHECK(7, 0xFE80FE8B);
}

MU_TEST(test_not_l)
{
    DATA(7, 0xFE800174);
    RUN("01000110 10 000111"); // NOT.l D7
    DATA_CHECK(7, 0x017FFE8B);
}

MU_TEST(test_tst_b_true)
{
    DATA(4, 0xFE800100);
    RUN("01001010 00 000100"); // TST.b D4
    mu_assert_int_eq(true, ZERO(m));
}

MU_TEST(test_tst_b_false)
{
    DATA(4, 0xFE80010C);
    RUN("01001010 00 000100"); // TST.b D4
    mu_assert_int_eq(false, ZERO(m));
}

MU_TEST(test_tst_w_true)
{
    DATA(4, 0xFAAF0000);
    RUN("01001010 01 000100"); // TST.w D4
    mu_assert_int_eq(true, ZERO(m));
}

MU_TEST(test_tst_w_false)
{
    DATA(4, 0xFAAF0FF0);
    RUN("01001010 01 000100"); // TST.w D4
    mu_assert_int_eq(false, ZERO(m));
}

MU_TEST(test_tst_l_true)
{
    DATA(4, 0);
    RUN("01001010 10 000100"); // TST.l D4
    mu_assert_int_eq(true, ZERO(m));
}

MU_TEST(test_tst_l_false)
{
    DATA(4, 0xF0000000);
    RUN("01001010 10 000100"); // TST.l D4
    mu_assert_int_eq(false, ZERO(m));
}

// TODO or, eor

MU_TEST_SUITE(test_suite_instructions_logic)
{
    MU_SUITE_CONFIGURE(&setup, &teardown);

    MU_RUN_TEST(test_and_b);
    MU_RUN_TEST(test_and_w);
    MU_RUN_TEST(test_and_l);

    MU_RUN_TEST(test_eor_b);
    MU_RUN_TEST(test_eor_w);
    MU_RUN_TEST(test_eor_l);

    MU_RUN_TEST(test_or_b);
    MU_RUN_TEST(test_or_w);
    MU_RUN_TEST(test_or_l);

    MU_RUN_TEST(test_not_b);
    MU_RUN_TEST(test_not_w);
    MU_RUN_TEST(test_not_l);

    MU_RUN_TEST(test_tst_b_true);
    MU_RUN_TEST(test_tst_b_false);
    MU_RUN_TEST(test_tst_w_true);
    MU_RUN_TEST(test_tst_w_false);
    MU_RUN_TEST(test_tst_l_true);
    MU_RUN_TEST(test_tst_l_false);
}
