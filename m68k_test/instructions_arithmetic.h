#pragma once

#include <minunit.h>
#include <m68k/m68k.h>

#include "globals.h"

MU_TEST(test_clr_b)
{
    DATA(3, 0x1234ABCD);
    RUN("01000010 00 000011"); // CLR.b D3
    DATA_CHECK(3, 0x1234AB00);
}

MU_TEST(test_clr_w)
{
    DATA(3, 0x1234ABCD);
    RUN("01000010 01 000011"); // CLR.w D3
    DATA_CHECK(3, 0x12340000);
}

MU_TEST(test_clr_l)
{
    DATA(3, 0x1234ABCD);
    RUN("01000010 10 000011"); // CLR.l D3
    DATA_CHECK(3, 0);
}

MU_TEST(test_ext_b_to_w_zero)
{
    DATA(7, 0xAA00);
    RUN("0100100 010 000 111"); // EXT.w D7
    DATA_CHECK(7, 0);
}

MU_TEST(test_ext_b_to_w_pos)
{
    DATA(7, 0xAA0A);
    RUN("0100100 010 000 111"); // EXT.w D7
    DATA_CHECK(7, 0xA);
}

MU_TEST(test_ext_b_to_w_neg)
{
    DATA(7, 0xAC);
    RUN("0100100 010 000 111"); // EXT.w D7
    DATA_CHECK(7, 0xFFAC);
}

MU_TEST(test_ext_w_to_l_zero)
{
    DATA(7, 0xAABC0000);
    RUN("0100100 011 000 111"); // EXT.l D7
    DATA_CHECK(7, 0);
}

MU_TEST(test_ext_w_to_l_pos)
{
    DATA(7, 0xAABC0010);
    RUN("0100100 011 000 111"); // EXT.l D7
    DATA_CHECK(7, 0x10);
}

MU_TEST(test_ext_w_to_l_neg)
{
    DATA(7, 0x9DFC);
    RUN("0100100 011 000 111"); // EXT.l D7
    DATA_CHECK(7, 0xFFFF9DFC);
}

MU_TEST_SUITE(test_suite_instructions_arithmetic)
{
    MU_SUITE_CONFIGURE(&setup, &teardown);

    MU_RUN_TEST(test_clr_b);
    MU_RUN_TEST(test_clr_w);
    MU_RUN_TEST(test_clr_l);

    MU_RUN_TEST(test_ext_b_to_w_zero);
    MU_RUN_TEST(test_ext_b_to_w_pos);
    MU_RUN_TEST(test_ext_b_to_w_neg);
    MU_RUN_TEST(test_ext_w_to_l_zero);
    MU_RUN_TEST(test_ext_w_to_l_pos);
    MU_RUN_TEST(test_ext_w_to_l_neg);
}
