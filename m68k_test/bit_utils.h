#pragma once

#include <minunit.h>
#include <m68k/bit_utils.h>

#include "globals.h"

MU_TEST(test_bit)
{
    mu_assert_int_eq_bin(0, BIT(0, 0));
    mu_assert_int_eq_bin(0, BIT(0, 5));

    mu_assert_int_eq_bin(1, BIT(1, 0));
    mu_assert_int_eq_bin(0, BIT(1, 1));

    mu_assert_int_eq_bin(1, BIT(0x80, 7));
    mu_assert_int_eq_bin(0, BIT(0x80, 6));
    mu_assert_int_eq_bin(0, BIT(0x80, 8));
}

MU_TEST(test_bit_set)
{
    mu_assert_int_eq_bin(1, BIT_SET(0, 0));
    mu_assert_int_eq_bin(1, BIT_SET(1, 0));

    mu_assert_int_eq_bin(8, BIT_SET(0, 3));
    mu_assert_int_eq_bin(9, BIT_SET(9, 3));
}

MU_TEST(test_bit_clr)
{
    mu_assert_int_eq_bin(0, BIT_CLR(0, 0));
    mu_assert_int_eq_bin(0, BIT_CLR(1, 0));

    mu_assert_int_eq_bin(0, BIT_CLR(0, 3));
    mu_assert_int_eq_bin(1, BIT_CLR(9, 3));

    mu_assert_int_eq_bin(0xDF, BIT_CLR(0xFF, 5));
}

MU_TEST(test_bit_chg)
{
    mu_assert_int_eq_bin(0, BIT_CHG(0, 0, 0));
    mu_assert_int_eq_bin(1, BIT_CHG(0, 0, 1));

    mu_assert_int_eq_bin(0, BIT_CHG(0, 3, 0));
    mu_assert_int_eq_bin(8, BIT_CHG(0, 3, 1));

    mu_assert_int_eq_bin(0xFE, BIT_CHG(0xFF, 0, 0));
    mu_assert_int_eq_bin(0xFF, BIT_CHG(0xFF, 0, 1));
    mu_assert_int_eq_bin(0x7F, BIT_CHG(0xFF, 7, 0));
}

MU_TEST(test_fragment)
{
    mu_assert_int_eq_bin(0, FRAGMENT(0, 0, 0));
    mu_assert_int_eq_bin(0, FRAGMENT(0, 7, 0));

    mu_assert_int_eq_bin(0, FRAGMENT(2, 0, 0));
    mu_assert_int_eq_bin(2, FRAGMENT(2, 1, 0));
    mu_assert_int_eq_bin(1, FRAGMENT(2, 1, 1));

    mu_assert_int_eq_bin(0xFF, FRAGMENT(0xFF, 7, 0));
    mu_assert_int_eq_bin(0xFF, FRAGMENT(0xFF, 20, 0));
    mu_assert_int_eq_bin(0x0F, FRAGMENT(0xFF, 3, 0));
    mu_assert_int_eq_bin(0x0F, FRAGMENT(0xFF, 7, 4));
}

MU_TEST(test_mask_below)
{
    mu_assert_int_eq_bin(0, MASK_BELOW(0, 0));
    mu_assert_int_eq_bin(1, MASK_BELOW(1, 0));
    mu_assert_int_eq_bin(3, MASK_BELOW(3, 0));
    mu_assert_int_eq_bin(2, MASK_BELOW(3, 1));
    mu_assert_int_eq_bin(0xFF, MASK_BELOW(0xFF, 0));
    mu_assert_int_eq_bin(0xF8, MASK_BELOW(0xFF, 3));
}

MU_TEST(test_mask_below_inc)
{
    mu_assert_int_eq_bin(0, MASK_BELOW_INC(0, 0));
    mu_assert_int_eq_bin(0, MASK_BELOW_INC(1, 0));
    mu_assert_int_eq_bin(2, MASK_BELOW_INC(3, 0));
    mu_assert_int_eq_bin(0, MASK_BELOW_INC(3, 1));
    mu_assert_int_eq_bin(0xFE, MASK_BELOW_INC(0xFF, 0));
    mu_assert_int_eq_bin(0xF0, MASK_BELOW_INC(0xFF, 3));
}

MU_TEST(test_mask_above)
{
    mu_assert_int_eq_bin(0, MASK_ABOVE(0, 0));
    mu_assert_int_eq_bin(1, MASK_ABOVE(1, 0));
    mu_assert_int_eq_bin(1, MASK_ABOVE(3, 0));
    mu_assert_int_eq_bin(3, MASK_ABOVE(3, 1));
    mu_assert_int_eq_bin(1, MASK_ABOVE(0xFF, 0));
    mu_assert_int_eq_bin(0x0F, MASK_ABOVE(0xFF, 3));
}

MU_TEST(test_mask_above_inc)
{
    mu_assert_int_eq_bin(0, MASK_ABOVE_INC(0, 0));
    mu_assert_int_eq_bin(0, MASK_ABOVE_INC(1, 0));
    mu_assert_int_eq_bin(0, MASK_ABOVE_INC(3, 0));
    mu_assert_int_eq_bin(1, MASK_ABOVE_INC(3, 1));
    mu_assert_int_eq_bin(1, MASK_ABOVE_INC(0xFF, 1));
    mu_assert_int_eq_bin(7, MASK_ABOVE_INC(0xFF, 3));
}

MU_TEST(test_bin_parse)
{
    mu_assert_int_eq_bin(0, bin_parse("0"));
    mu_assert_int_eq_bin(0, bin_parse("000"));
    mu_assert_int_eq_bin(0, bin_parse("0 0    0"));
    mu_assert_int_eq_bin(0, bin_parse(""));
    mu_assert_int_eq_bin(0, bin_parse("   "));

    mu_assert_int_eq_bin(1, bin_parse("1"));
    mu_assert_int_eq_bin(1, bin_parse("00 01"));

    mu_assert_int_eq_bin(3, bin_parse("11"));
    mu_assert_int_eq_bin(3, bin_parse("     1   1     "));

    mu_assert_int_eq_bin(0xCC, bin_parse("11001100"));
    mu_assert_int_eq_bin(0xE3, bin_parse("1110 0011"));
    mu_assert_int_eq_bin(0xFF, bin_parse("1111 1111"));
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
