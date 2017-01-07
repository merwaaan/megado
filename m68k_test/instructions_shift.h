#pragma once

#include <minunit.h>
#include <m68k/m68k.h>

#include "globals.h"

MU_TEST(test_lsl) // TODO test immediate version
{
    DATA(7, 1);  DATA(0, 0xFFFF);
    RUN("1110001 1 11 000000"); // LSL D7, D0 (1 bit)
    DATA_CHECK(0, 0xFFFE);
}

MU_TEST(test_lsl_zero)
{
    DATA(7, 0);  DATA(0, 0xFFFF);
    RUN("1110001 1 11 000000"); // LSL D7, D0 (0 bits)
    DATA_CHECK(0, 0xFFFF);
}

/*MU_TEST(test_lsl)
{
    DATA(7, 0);  DATA(0, 0xFFFF);
    RUN("1110001 1 11 000000"); // LSL D7, D0
    DATA_CHECK(0, 0xFFFF);
}*/

MU_TEST(test_swap)
{
    DATA(4, 0x1234ABCD);
    RUN("0100100001000 100"); // SWAP D4
    DATA_CHECK(4, 0xABCD1234);
}

MU_TEST_SUITE(test_suite_instructions_shift)
{
    MU_SUITE_CONFIGURE(&setup, &teardown);

    //MU_RUN_TEST(test_lsl);
    //MU_RUN_TEST(test_lsl_zero);

    MU_RUN_TEST(test_swap);
}
