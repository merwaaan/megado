#pragma once

#include <minunit.h>
#include <m68k/m68k.h>

#include "globals.h"

MU_TEST(test_clr_b)
{
    DATA(3, 0x1234ABCD);
    RUN("01000010 00 000011"); // CLR.b 3
    DATA_CHECK(3, 0x1234AB00);
}

MU_TEST(test_clr_w)
{
    DATA(3, 0x1234ABCD);
    RUN("01000010 01 000011"); // CLR.w 3
    DATA_CHECK(3, 0x12340000);
}

MU_TEST(test_clr_l)
{
    DATA(3, 0x1234ABCD);
    RUN("01000010 10 000011"); // CLR.l 3
    DATA_CHECK(3, 0);
}

MU_TEST_SUITE(test_suite_instructions_arithmetic)
{
    MU_SUITE_CONFIGURE(&setup, &teardown);

    MU_RUN_TEST(test_clr_b);
    MU_RUN_TEST(test_clr_w);
    MU_RUN_TEST(test_clr_l);
}
