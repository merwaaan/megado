#pragma once

#include <minunit.h>

#include <m68k/bit_utils.h>

MU_TEST(test_bit)
{
    mu_assert_int_eq(0, BIT(0, 0));
    mu_assert_int_eq(0, BIT(0, 5));

    mu_assert_int_eq(1, BIT(1, 0));
    mu_assert_int_eq(0, BIT(1, 1));

    mu_assert_int_eq(1, BIT(0x80, 7));
    mu_assert_int_eq(0, BIT(0x80, 6));
    mu_assert_int_eq(0, BIT(0x80, 8));
}

MU_TEST(test_fragment)
{
    mu_assert_int_eq(0, FRAGMENT(0, 0, 0));
    mu_assert_int_eq(0, FRAGMENT(0, 7, 0));

    mu_assert_int_eq(0, FRAGMENT(2, 0, 0));
    mu_assert_int_eq(2, FRAGMENT(2, 1, 0));
    mu_assert_int_eq(1, FRAGMENT(2, 1, 1));

    mu_assert_int_eq(0xFF, FRAGMENT(0xFF, 7, 0));
    mu_assert_int_eq(0xFF, FRAGMENT(0xFF, 100, 0));
    mu_assert_int_eq(0x0F, FRAGMENT(0xFF, 3, 0));
    mu_assert_int_eq(0x0F, FRAGMENT(0xFF, 7, 4));
}

MU_TEST(test_mask_below)
{
    /*mu_assert_int_eq(0, MASK_BELOW(0, 0));
    mu_assert_int_eq(0, MASK_BELOW(1, 0));
    mu_assert_int_eq(2, MASK_BELOW(3, 0));
    mu_assert_int_eq(0, MASK_BELOW(3, 1));
    mu_assert_int_eq(0xFE, MASK_BELOW(0xFF, 0));
    mu_assert_int_eq(0xF0, MASK_BELOW(0xFF, 3));*/
}

MU_TEST(test_mask_below_inc)
{
    /*mu_assert_int_eq(0, MASK_BELOW_INC(0, 0));
    mu_assert_int_eq(1, MASK_BELOW_INC(1, 0));
    mu_assert_int_eq(3, MASK_BELOW_INC(3, 0));
    mu_assert_int_eq(2, MASK_BELOW_INC(3, 1));
    mu_assert_int_eq(0xFF, MASK_BELOW_INC(0xFF, 0));
    mu_assert_int_eq(0xF8, MASK_BELOW_INC(0xFF, 3));*/
}

MU_TEST(test_mask_above)
{
}

MU_TEST(test_mask_above_inc)
{
}

MU_TEST(test_parse_bin)
{
    mu_assert_int_eq(0, parse_bin("0"));
    mu_assert_int_eq(0, parse_bin("000"));
    mu_assert_int_eq(0, parse_bin("0 0    0"));
    mu_assert_int_eq(0, parse_bin(""));
    mu_assert_int_eq(0, parse_bin("   "));

    mu_assert_int_eq(1, parse_bin("1"));
    mu_assert_int_eq(1, parse_bin("00 01"));

    mu_assert_int_eq(3, parse_bin("11"));
    mu_assert_int_eq(3, parse_bin("     1   1     "));

    mu_assert_int_eq(0xCC, parse_bin("11001100"));
    mu_assert_int_eq(0xE3, parse_bin("1110 0011"));
    mu_assert_int_eq(0xFF, parse_bin("1111 1111"));
}

MU_TEST_SUITE(test_suite_bit_utils)
{
    MU_RUN_TEST(test_bit);
    MU_RUN_TEST(test_fragment);
    MU_RUN_TEST(test_mask_below);
    MU_RUN_TEST(test_mask_below_inc);
    MU_RUN_TEST(test_mask_above);
    MU_RUN_TEST(test_mask_above_inc);
    MU_RUN_TEST(test_parse_bin);
}