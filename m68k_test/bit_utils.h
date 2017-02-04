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

MU_TEST(test_sign_bit)
{
    ASSERT_BIN(0, SIGN_BIT(0, Byte));
    ASSERT_BIN(0, SIGN_BIT(1, Byte));
    ASSERT_BIN(0, SIGN_BIT(0x40, Byte));
    ASSERT_BIN(1, SIGN_BIT(0x80, Byte));
    ASSERT_BIN(1, SIGN_BIT(0xFE, Byte));
    ASSERT_BIN(1, SIGN_BIT(0xFF, Byte));

    ASSERT_BIN(0, SIGN_BIT(0, Word));
    ASSERT_BIN(0, SIGN_BIT(0x80, Word));
    ASSERT_BIN(0, SIGN_BIT(0xFF, Word));
    ASSERT_BIN(0, SIGN_BIT(0x7FFF, Word));
    ASSERT_BIN(1, SIGN_BIT(0x8000, Word));
    ASSERT_BIN(1, SIGN_BIT(0xFFFF, Word));

    ASSERT_BIN(0, SIGN_BIT(0, Long));
    ASSERT_BIN(0, SIGN_BIT(0xFF, Long));
    ASSERT_BIN(0, SIGN_BIT(0xFFFF, Long));
    ASSERT_BIN(0, SIGN_BIT(0x7FFFFFFF, Long));
    ASSERT_BIN(1, SIGN_BIT(0x80000000, Long));
    ASSERT_BIN(1, SIGN_BIT(0xFFFFFFFF, Long));
}

MU_TEST(test_carry_add)
{
    ASSERT_BIN(false, CHECK_CARRY_ADD(0, 0, Byte));
    ASSERT_BIN(false, CHECK_CARRY_ADD(0xFF, 0, Byte));
    ASSERT_BIN(false, CHECK_CARRY_ADD(0, 0xFF, Byte));
    ASSERT_BIN(false, CHECK_CARRY_ADD(0x10, 0xA0, Byte));
    ASSERT_BIN(false, CHECK_CARRY_ADD(0xA0, 0x10, Byte));
    ASSERT_BIN(true, CHECK_CARRY_ADD(0xFF, 1, Byte));
    ASSERT_BIN(true, CHECK_CARRY_ADD(1, 0xFF, Byte));
    ASSERT_BIN(true, CHECK_CARRY_ADD(0xA0, 0xA0, Byte));
    ASSERT_BIN(true, CHECK_CARRY_ADD(0xFF, 0xFF, Byte));

    ASSERT_BIN(false, CHECK_CARRY_ADD(0xFF, 0xFF, Word));
    ASSERT_BIN(false, CHECK_CARRY_ADD(0xF000, 0xFF, Word));
    ASSERT_BIN(true, CHECK_CARRY_ADD(0xFFFF, 1, Word));
    ASSERT_BIN(true, CHECK_CARRY_ADD(0xFFFF, 0xFFFF, Word));

    ASSERT_BIN(false, CHECK_CARRY_ADD(0xFF, 0xFF, Long));
    ASSERT_BIN(false, CHECK_CARRY_ADD(0xFFFF, 0xFFFF, Long));
    ASSERT_BIN(false, CHECK_CARRY_ADD(0xFFFF000, 0xFF, Long));
    ASSERT_BIN(true, CHECK_CARRY_ADD(0xFFFFFFFF, 1, Long));
    ASSERT_BIN(true, CHECK_CARRY_ADD(0xFFFFFFFF, 0xFFFF, Long));
}

MU_TEST(test_carry_sub)
{
    ASSERT_BIN(false, CHECK_CARRY_SUB(0, 0, Byte));
    ASSERT_BIN(false, CHECK_CARRY_SUB(0xFF, 0, Byte));
    ASSERT_BIN(true, CHECK_CARRY_SUB(0, 1, Byte));
    ASSERT_BIN(true, CHECK_CARRY_SUB(0, 0xFF, Byte));
    ASSERT_BIN(true, CHECK_CARRY_SUB(0x10, 0xA0, Byte));
    ASSERT_BIN(false, CHECK_CARRY_SUB(0xA0, 0x10, Byte));
    ASSERT_BIN(false, CHECK_CARRY_SUB(0xFF, 1, Byte));
    ASSERT_BIN(true, CHECK_CARRY_SUB(1, 0xFF, Byte));
    ASSERT_BIN(false, CHECK_CARRY_SUB(0xA0, 0xA0, Byte));
    ASSERT_BIN(false, CHECK_CARRY_SUB(0xFF, 0xFF, Byte));

    ASSERT_BIN(false, CHECK_CARRY_SUB(0xFF, 0xFF, Word));
    ASSERT_BIN(false, CHECK_CARRY_SUB(0xF000, 0xFF, Word));
    ASSERT_BIN(false, CHECK_CARRY_SUB(0xFFFF, 1, Word));
    ASSERT_BIN(false, CHECK_CARRY_SUB(0xFFFF, 0xFFFF, Word));
    ASSERT_BIN(true, CHECK_CARRY_SUB(0, 0xFFFF, Word));
    ASSERT_BIN(true, CHECK_CARRY_SUB(0, 1, Word));
    ASSERT_BIN(true, CHECK_CARRY_SUB(0xAAAA, 0xAAAB, Word));

    ASSERT_BIN(false, CHECK_CARRY_SUB(0xFF, 0xFF, Long));
    ASSERT_BIN(false, CHECK_CARRY_SUB(0xFFFF, 0xFFFF, Long));
    ASSERT_BIN(false, CHECK_CARRY_SUB(0xFFFF000, 0xFF, Long));
    ASSERT_BIN(false, CHECK_CARRY_SUB(0xFFFFFFFF, 1, Long));
    ASSERT_BIN(false, CHECK_CARRY_SUB(0xFFFFFFFF, 0xFFFF, Long));
    ASSERT_BIN(true, CHECK_CARRY_SUB(0, 0xFFFF, Long));
    ASSERT_BIN(true, CHECK_CARRY_SUB(0xF0000000, 0xFFFFFFFF, Long));
}

MU_TEST(test_overflow_add)
{
    ASSERT_BIN(false, CHECK_OVERFLOW_ADD(0, 0, Byte));
    ASSERT_BIN(false, CHECK_OVERFLOW_ADD(0, 0x50, Byte));
    ASSERT_BIN(false, CHECK_OVERFLOW_ADD(0x50, 0, Byte));
    ASSERT_BIN(true, CHECK_OVERFLOW_ADD(0x50, 0x50, Byte));
    ASSERT_BIN(true, CHECK_OVERFLOW_ADD(0x7F, 1, Byte));
    ASSERT_BIN(false, CHECK_OVERFLOW_ADD(0x80, 0, Byte));
    ASSERT_BIN(false, CHECK_OVERFLOW_ADD(0x80, 1, Byte));
    ASSERT_BIN(false, CHECK_OVERFLOW_ADD(0x80, 0x50, Byte));
    ASSERT_BIN(true, CHECK_OVERFLOW_ADD(0x80, 0x80, Byte));
    ASSERT_BIN(false, CHECK_OVERFLOW_ADD(0xFF, 1, Byte));

    ASSERT_BIN(false, CHECK_OVERFLOW_ADD(0, 0, Word));
    ASSERT_BIN(false, CHECK_OVERFLOW_ADD(0x7F, 1,Word));
    ASSERT_BIN(true, CHECK_OVERFLOW_ADD(0x7FFF, 1, Word));
    ASSERT_BIN(false, CHECK_OVERFLOW_ADD(0xFFFF, 0, Word));
    ASSERT_BIN(false, CHECK_OVERFLOW_ADD(0xFFFF, 1, Word));
    ASSERT_BIN(true, CHECK_OVERFLOW_ADD(0x5000, 0x5000, Word));

    ASSERT_BIN(false, CHECK_OVERFLOW_ADD(0, 0, Long));
    ASSERT_BIN(false, CHECK_OVERFLOW_ADD(0x7F, 1, Long));
    ASSERT_BIN(false, CHECK_OVERFLOW_ADD(0x7FFF, 1, Long));
    ASSERT_BIN(true, CHECK_OVERFLOW_ADD(0x7FFFFFFF, 1, Long));
    ASSERT_BIN(false, CHECK_OVERFLOW_ADD(0x7FFFFFFF, 0, Long));
    ASSERT_BIN(false, CHECK_OVERFLOW_ADD(0xFFFFFFFF, 0, Long));
    ASSERT_BIN(false, CHECK_OVERFLOW_ADD(0xFFFFFFFF, 1, Long));
    ASSERT_BIN(true, CHECK_OVERFLOW_ADD(0x50000000, 0x50000000, Long));
}

MU_TEST(test_overflow_sub)
{
    ASSERT_BIN(false, CHECK_OVERFLOW_SUB(0, 0, Byte));
    ASSERT_BIN(false, CHECK_OVERFLOW_SUB(0, 0x50, Byte));
    ASSERT_BIN(false, CHECK_OVERFLOW_SUB(0x50, 0, Byte));
    ASSERT_BIN(false, CHECK_OVERFLOW_SUB(0x50, 0x50, Byte));
    ASSERT_BIN(false, CHECK_OVERFLOW_SUB(0x7F, 1, Byte));
    ASSERT_BIN(false, CHECK_OVERFLOW_SUB(0x80, 0, Byte));
    ASSERT_BIN(true, CHECK_OVERFLOW_SUB(0x80, 1, Byte));
    ASSERT_BIN(true, CHECK_OVERFLOW_SUB(0x80, 0x50, Byte));
    ASSERT_BIN(false, CHECK_OVERFLOW_SUB(0x80, 0x80, Byte));
    ASSERT_BIN(false, CHECK_OVERFLOW_SUB(0xFF, 1, Byte));

    ASSERT_BIN(false, CHECK_OVERFLOW_SUB(0, 0, Word));
    ASSERT_BIN(false, CHECK_OVERFLOW_SUB(0x7F, 1, Word));
    ASSERT_BIN(false, CHECK_OVERFLOW_SUB(0x7FFF, 1, Word));
    ASSERT_BIN(false, CHECK_OVERFLOW_SUB(0xFFFF, 0, Word));
    ASSERT_BIN(false, CHECK_OVERFLOW_SUB(0xFFFF, 1, Word));
    ASSERT_BIN(false, CHECK_OVERFLOW_SUB(0xFFFF, 0x8001, Word));
    ASSERT_BIN(true, CHECK_OVERFLOW_SUB(0x1000, 0x8000, Word));
    ASSERT_BIN(true, CHECK_OVERFLOW_SUB(0x8000, 0x2000, Word));

    ASSERT_BIN(false, CHECK_OVERFLOW_SUB(0, 0, Long));
    ASSERT_BIN(false, CHECK_OVERFLOW_SUB(0x7F, 1, Long));
    ASSERT_BIN(false, CHECK_OVERFLOW_SUB(0x7FFF, 1, Long));
    ASSERT_BIN(false, CHECK_OVERFLOW_SUB(0x7FFFFFFF, 0, Long));
    ASSERT_BIN(false, CHECK_OVERFLOW_SUB(0x7FFFFFFF, 1, Long));
    ASSERT_BIN(false, CHECK_OVERFLOW_SUB(0xFFFFFFFF, 0, Long));
    ASSERT_BIN(false, CHECK_OVERFLOW_SUB(0xFFFFFFFF, 1, Long));
    ASSERT_BIN(true, CHECK_OVERFLOW_SUB(0x80000000, 0x1, Long));
    ASSERT_BIN(true, CHECK_OVERFLOW_SUB(0x80000000, 0x7FFFFFFF, Long));
    ASSERT_BIN(true, CHECK_OVERFLOW_SUB(0x10000000, 0x90000000, Long));
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
{
    char buffer[100];
    mu_assert_str_eq("0", bin_tostring(0, buffer));
    mu_assert_str_eq("1", bin_tostring(1, buffer));
    mu_assert_str_eq("10", bin_tostring(2, buffer));
    mu_assert_str_eq("11111111", bin_tostring(0xFF, buffer));
    mu_assert_str_eq("11111110", bin_tostring(0xFE, buffer));
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

    MU_RUN_TEST(test_sign_bit);

    MU_RUN_TEST(test_carry_add);
    MU_RUN_TEST(test_carry_sub);
    MU_RUN_TEST(test_overflow_add);
    MU_RUN_TEST(test_overflow_sub);

    MU_RUN_TEST(test_bin_parse);
    MU_RUN_TEST(test_bin_tostring);
}
