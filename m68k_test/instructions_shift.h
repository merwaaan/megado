#pragma once

#include <minunit.h>
#include <m68k/m68k.h>

#include "globals.h"

MU_TEST(test_swap)
{
    DATA(4, 0x1234ABCD);
    RUN("0100100001000 100"); // SWAP D4
    DATA_CHECK(4, 0xABCD1234);
}

MU_TEST_SUITE(test_suite_instructions_shift)
{
    MU_SUITE_CONFIGURE(&setup, &teardown);

    MU_RUN_TEST(test_swap);
}
