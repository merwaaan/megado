#pragma once

#include <minunit.h>
#include <m68k/bit_utils.h>

#include "globals.h"

MU_TEST(test_bit)
{
    ASSERT_BIN(0, BIT(0, 0));
    ASSERT_BIN(0, BIT(0, 5));

    ASSERT_BIN(1, BIT(1, 0));
    ASSERT_BIN(0, BIT(1, 1));

    ASSERT_BIN(1, BIT(0x80, 7));
    ASSERT_BIN(0, BIT(0x80, 6));
    ASSERT_BIN(0, BIT(0x80, 8));
}

MU_TEST(test_bit_set)
{
    ASSERT_BIN(1, BIT_SET(0, 0));
    ASSERT_BIN(1, BIT_SET(1, 0));

    ASSERT_BIN(8, BIT_SET(0, 3));
    ASSERT_BIN(9, BIT_SET(9, 3));
}

MU_TEST(test_bit_clr)
{
    ASSERT_BIN(0, BIT_CLR(0, 0));
    ASSERT_BIN(0, BIT_CLR(1, 0));

    ASSERT_BIN(0, BIT_CLR(0, 3));
    ASSERT_BIN(1, BIT_CLR(9, 3));

    ASSERT_BIN(0xDF, BIT_CLR(0xFF, 5));
}

MU_TEST(test_bit_chg)
{
    ASSERT_BIN(0, BIT_CHG(0, 0, 0));
    ASSERT_BIN(1, BIT_CHG(0, 0, 1));

    ASSERT_BIN(0, BIT_CHG(0, 3, 0));
    ASSERT_BIN(8, BIT_CHG(0, 3, 1));

    ASSERT_BIN(0xFE, BIT_CHG(0xFF, 0, 0));
    ASSERT_BIN(0xFF, BIT_CHG(0xFF, 0, 1));
    ASSERT_BIN(0x7F, BIT_CHG(0xFF, 7, 0));
}

MU_TEST(test_fragment)
{
    ASSERT_BIN(0, FRAGMENT(0, 0, 0));
    ASSERT_BIN(0, FRAGMENT(0, 7, 0));

    ASSERT_BIN(1, FRAGMENT(1, 0, 0));
    ASSERT_BIN(0, FRAGMENT(1, 7, 1));
    ASSERT_BIN(1, FRAGMENT(1, 7, 0));

    ASSERT_BIN(0, FRAGMENT(2, 0, 0));
    ASSERT_BIN(2, FRAGMENT(2, 1, 0));
    ASSERT_BIN(1, FRAGMENT(2, 1, 1));

    ASSERT_BIN(0xFF, FRAGMENT(0xFF, 7, 0));
    ASSERT_BIN(0xFF, FRAGMENT(0xFF, 20, 0));
    ASSERT_BIN(0x0F, FRAGMENT(0xFF, 3, 0));
    ASSERT_BIN(0x0F, FRAGMENT(0xFF, 7, 4));

    ASSERT_BIN(1, FRAGMENT(0xFFFFFFFF, 0, 0));
    ASSERT_BIN(0xFF, FRAGMENT(0xFFFFFFFF, 7, 0));
    ASSERT_BIN(0xFFFF, FRAGMENT(0xFFFFFFFF, 15, 0));
    ASSERT_BIN(0xFF, FRAGMENT(0xFFFFFFFF, 15, 8));
    ASSERT_BIN(1, FRAGMENT(0xFFFFFFFF, 31, 31));
    ASSERT_BIN(0xFFFFFFFF, FRAGMENT(0xFFFFFFFF, 31, 0));
}

MU_TEST(test_mask_below)
{
    ASSERT_BIN(0, MASK_BELOW(0, 0));
    ASSERT_BIN(1, MASK_BELOW(1, 0));
    ASSERT_BIN(3, MASK_BELOW(3, 0));
    ASSERT_BIN(2, MASK_BELOW(3, 1));
    ASSERT_BIN(0xFF, MASK_BELOW(0xFF, 0));
    ASSERT_BIN(0xF8, MASK_BELOW(0xFF, 3));
}

MU_TEST(test_mask_below_inc)
{
    ASSERT_BIN(0, MASK_BELOW_INC(0, 0));
    ASSERT_BIN(0, MASK_BELOW_INC(1, 0));
    ASSERT_BIN(2, MASK_BELOW_INC(3, 0));
    ASSERT_BIN(0, MASK_BELOW_INC(3, 1));
    ASSERT_BIN(0xFE, MASK_BELOW_INC(0xFF, 0));
    ASSERT_BIN(0xF0, MASK_BELOW_INC(0xFF, 3));
}

MU_TEST(test_mask_above)
{
    ASSERT_BIN(0, MASK_ABOVE(0, 0));
    ASSERT_BIN(1, MASK_ABOVE(1, 0));
    ASSERT_BIN(1, MASK_ABOVE(3, 0));
    ASSERT_BIN(3, MASK_ABOVE(3, 1));
    ASSERT_BIN(1, MASK_ABOVE(0xFF, 0));
    ASSERT_BIN(0x0F, MASK_ABOVE(0xFF, 3));
}

MU_TEST(test_mask_above_inc)
{
    ASSERT_BIN(0, MASK_ABOVE_INC(0, 0));
    ASSERT_BIN(0, MASK_ABOVE_INC(1, 0));
    ASSERT_BIN(0, MASK_ABOVE_INC(3, 0));
    ASSERT_BIN(1, MASK_ABOVE_INC(3, 1));
    ASSERT_BIN(1, MASK_ABOVE_INC(0xFF, 1));
    ASSERT_BIN(7, MASK_ABOVE_INC(0xFF, 3));
}

MU_TEST(test_bin_parse)
{
    ASSERT_BIN(0, bin_parse("0"));
    ASSERT_BIN(0, bin_parse("000"));
    ASSERT_BIN(0, bin_parse("0 0    0"));
    ASSERT_BIN(0, bin_parse(""));
    ASSERT_BIN(0, bin_parse("   "));

    ASSERT_BIN(1, bin_parse("1"));
    ASSERT_BIN(1, bin_parse("00 01"));

    ASSERT_BIN(3, bin_parse("11"));
    ASSERT_BIN(3, bin_parse("     1   1     "));

    ASSERT_BIN(0xCC, bin_parse("11001100"));
    ASSERT_BIN(0xE3, bin_parse("1110 0011"));
    ASSERT_BIN(0xFF, bin_parse("1111 1111"));
}

MU_TEST(test_bin_tostring)
{;
    mu_assert_str_eq("0", bin_tostring(0));
    mu_assert_str_eq("1", bin_tostring(1));
    mu_assert_str_eq("10", bin_tostring(2));
    mu_assert_str_eq("11111111", bin_tostring(0xFF));
    mu_assert_str_eq("11111110", bin_tostring(0xFE));
}

MU_TEST_SUITE(test_suite_bit_utils)
{
    MU_RUN_TEST(test_bit);
    MU_RUN_TEST(test_bit_set);
    MU_RUN_TEST(test_bit_clr);
    MU_RUN_TEST(test_bit_chg);

    MU_RUN_TEST(test_fragment);

    MU_RUN_TEST(test_mask_below);
    MU_RUN_TEST(test_mask_below_inc);
    MU_RUN_TEST(test_mask_above);
    MU_RUN_TEST(test_mask_above_inc);

    MU_RUN_TEST(test_bin_parse);
    MU_RUN_TEST(test_bin_tostring);
}
