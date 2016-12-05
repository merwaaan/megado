#pragma once

#include <minunit.h>
#include <m68k/m68k.h>

#include "globals.h"

MU_TEST(test_bchg)
{
}

MU_TEST(test_bclr)
{
}

MU_TEST(test_bset)
{
}

MU_TEST(test_btst)
{
}

MU_TEST_SUITE(test_suite_instructions_bit)
{
    MU_SUITE_CONFIGURE(&setup, &teardown);

    MU_RUN_TEST(test_bchg);
    MU_RUN_TEST(test_bclr);
    MU_RUN_TEST(test_bset);
    MU_RUN_TEST(test_btst);
}
