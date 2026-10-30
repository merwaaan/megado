#include <m68k/bit_utils.h>

#include <greatest.h>

#include "globals.h"

TEST test_bit()
{
    CHECK(0, BIT(0, 0));
    CHECK(0, BIT(0, 5));

    CHECK(1, BIT(1, 0));
    CHECK(0, BIT(1, 1));

    CHECK(1, BIT(0x80, 7));
    CHECK(0, BIT(0x80, 6));
    CHECK(0, BIT(0x80, 8));
    PASS();
}

TEST test_bit_set()
{
    CHECK(1, BIT_SET(0, 0));
    CHECK(1, BIT_SET(1, 0));

    CHECK(8, BIT_SET(0, 3));
    CHECK(9, BIT_SET(9, 3));
    PASS();
}

TEST test_bit_clr()
{
    CHECK(0, BIT_CLR(0, 0));
    CHECK(0, BIT_CLR(1, 0));

    CHECK(0, BIT_CLR(0, 3));
    CHECK(1, BIT_CLR(9, 3));

    CHECK(0xDF, BIT_CLR(0xFF, 5));
    PASS();
}

TEST test_bit_chg()
{
    CHECK(0, BIT_CHG(0, 0, 0));
    CHECK(1, BIT_CHG(0, 0, 1));

    CHECK(0, BIT_CHG(0, 3, 0));
    CHECK(8, BIT_CHG(0, 3, 1));

    CHECK(0xFE, BIT_CHG(0xFF, 0, 0));
    CHECK(0xFF, BIT_CHG(0xFF, 0, 1));
    CHECK(0x7F, BIT_CHG(0xFF, 7, 0));
    PASS();
}

TEST test_fragment()
{
    CHECK(0, FRAGMENT(0, 0, 0));
    CHECK(0, FRAGMENT(0, 7, 0));

    CHECK(1, FRAGMENT(1, 0, 0));
    CHECK(0, FRAGMENT(1, 7, 1));
    CHECK(1, FRAGMENT(1, 7, 0));

    CHECK(0, FRAGMENT(2, 0, 0));
    CHECK(2, FRAGMENT(2, 1, 0));
    CHECK(1, FRAGMENT(2, 1, 1));

    CHECK(0xFF, FRAGMENT(0xFF, 7, 0));
    CHECK(0xFF, FRAGMENT(0xFF, 20, 0));
    CHECK(0x0F, FRAGMENT(0xFF, 3, 0));
    CHECK(0x0F, FRAGMENT(0xFF, 7, 4));

    CHECK(1, FRAGMENT(0xFFFFFFFF, 0, 0));
    CHECK(0xFF, FRAGMENT(0xFFFFFFFF, 7, 0));
    CHECK(0xFFFF, FRAGMENT(0xFFFFFFFF, 15, 0));
    CHECK(0xFF, FRAGMENT(0xFFFFFFFF, 15, 8));
    CHECK(1, FRAGMENT(0xFFFFFFFF, 31, 31));
    CHECK(0xFFFFFFFF, FRAGMENT(0xFFFFFFFF, 31, 0));
    PASS();
}

TEST test_mask_below()
{
    CHECK(0, MASK_BELOW(0, 0));
    CHECK(1, MASK_BELOW(1, 0));
    CHECK(3, MASK_BELOW(3, 0));
    CHECK(2, MASK_BELOW(3, 1));
    CHECK(0xFF, MASK_BELOW(0xFF, 0));
    CHECK(0xF8, MASK_BELOW(0xFF, 3));
    PASS();
}

TEST test_mask_below_inc()
{
    CHECK(0, MASK_BELOW_INC(0, 0));
    CHECK(0, MASK_BELOW_INC(1, 0));
    CHECK(2, MASK_BELOW_INC(3, 0));
    CHECK(0, MASK_BELOW_INC(3, 1));
    CHECK(0xFE, MASK_BELOW_INC(0xFF, 0));
    CHECK(0xF0, MASK_BELOW_INC(0xFF, 3));
    PASS();
}

TEST test_mask_above()
{
    CHECK(0, MASK_ABOVE(0, 0));
    CHECK(1, MASK_ABOVE(1, 0));
    CHECK(1, MASK_ABOVE(3, 0));
    CHECK(3, MASK_ABOVE(3, 1));
    CHECK(1, MASK_ABOVE(0xFF, 0));
    CHECK(0x0F, MASK_ABOVE(0xFF, 3));
    PASS();
}

TEST test_mask_above_inc()
{
    CHECK(0, MASK_ABOVE_INC(0, 0));
    CHECK(0, MASK_ABOVE_INC(1, 0));
    CHECK(0, MASK_ABOVE_INC(3, 0));
    CHECK(1, MASK_ABOVE_INC(3, 1));
    CHECK(1, MASK_ABOVE_INC(0xFF, 1));
    CHECK(7, MASK_ABOVE_INC(0xFF, 3));
    PASS();
}

TEST test_sign_bit()
{
    CHECK(0, SIGN_BIT(0, Byte));
    CHECK(0, SIGN_BIT(1, Byte));
    CHECK(0, SIGN_BIT(0x40, Byte));
    CHECK(1, SIGN_BIT(0x80, Byte));
    CHECK(1, SIGN_BIT(0xFE, Byte));
    CHECK(1, SIGN_BIT(0xFF, Byte));

    CHECK(0, SIGN_BIT(0, Word));
    CHECK(0, SIGN_BIT(0x80, Word));
    CHECK(0, SIGN_BIT(0xFF, Word));
    CHECK(0, SIGN_BIT(0x7FFF, Word));
    CHECK(1, SIGN_BIT(0x8000, Word));
    CHECK(1, SIGN_BIT(0xFFFF, Word));

    CHECK(0, SIGN_BIT(0, Long));
    CHECK(0, SIGN_BIT(0xFF, Long));
    CHECK(0, SIGN_BIT(0xFFFF, Long));
    CHECK(0, SIGN_BIT(0x7FFFFFFF, Long));
    CHECK(1, SIGN_BIT(0x80000000, Long));
    CHECK(1, SIGN_BIT(0xFFFFFFFF, Long));
    PASS();
}

TEST test_carry_add()
{
    CHECK(false, CHECK_CARRY_ADD(0, 0, Byte));
    CHECK(false, CHECK_CARRY_ADD(0xFF, 0, Byte));
    CHECK(false, CHECK_CARRY_ADD(0, 0xFF, Byte));
    CHECK(false, CHECK_CARRY_ADD(0x10, 0xA0, Byte));
    CHECK(false, CHECK_CARRY_ADD(0xA0, 0x10, Byte));
    CHECK(true, CHECK_CARRY_ADD(0xFF, 1, Byte));
    CHECK(true, CHECK_CARRY_ADD(1, 0xFF, Byte));
    CHECK(true, CHECK_CARRY_ADD(0xA0, 0xA0, Byte));
    CHECK(true, CHECK_CARRY_ADD(0xFF, 0xFF, Byte));

    CHECK(false, CHECK_CARRY_ADD(0xFF, 0xFF, Word));
    CHECK(false, CHECK_CARRY_ADD(0xF000, 0xFF, Word));
    CHECK(true, CHECK_CARRY_ADD(0xFFFF, 1, Word));
    CHECK(true, CHECK_CARRY_ADD(0xFFFF, 0xFFFF, Word));

    CHECK(false, CHECK_CARRY_ADD(0xFF, 0xFF, Long));
    CHECK(false, CHECK_CARRY_ADD(0xFFFF, 0xFFFF, Long));
    CHECK(false, CHECK_CARRY_ADD(0xFFFF000, 0xFF, Long));
    CHECK(true, CHECK_CARRY_ADD(0xFFFFFFFF, 1, Long));
    CHECK(true, CHECK_CARRY_ADD(0xFFFFFFFF, 0xFFFF, Long));
    PASS();
}

TEST test_carry_sub()
{
    CHECK(false, CHECK_CARRY_SUB(0, 0, Byte));
    CHECK(false, CHECK_CARRY_SUB(0xFF, 0, Byte));
    CHECK(true, CHECK_CARRY_SUB(0, 1, Byte));
    CHECK(true, CHECK_CARRY_SUB(0, 0xFF, Byte));
    CHECK(true, CHECK_CARRY_SUB(0x10, 0xA0, Byte));
    CHECK(false, CHECK_CARRY_SUB(0xA0, 0x10, Byte));
    CHECK(false, CHECK_CARRY_SUB(0xFF, 1, Byte));
    CHECK(true, CHECK_CARRY_SUB(1, 0xFF, Byte));
    CHECK(false, CHECK_CARRY_SUB(0xA0, 0xA0, Byte));
    CHECK(false, CHECK_CARRY_SUB(0xFF, 0xFF, Byte));

    CHECK(false, CHECK_CARRY_SUB(0xFF, 0xFF, Word));
    CHECK(false, CHECK_CARRY_SUB(0xF000, 0xFF, Word));
    CHECK(false, CHECK_CARRY_SUB(0xFFFF, 1, Word));
    CHECK(false, CHECK_CARRY_SUB(0xFFFF, 0xFFFF, Word));
    CHECK(true, CHECK_CARRY_SUB(0, 0xFFFF, Word));
    CHECK(true, CHECK_CARRY_SUB(0, 1, Word));
    CHECK(true, CHECK_CARRY_SUB(0xAAAA, 0xAAAB, Word));

    CHECK(false, CHECK_CARRY_SUB(0xFF, 0xFF, Long));
    CHECK(false, CHECK_CARRY_SUB(0xFFFF, 0xFFFF, Long));
    CHECK(false, CHECK_CARRY_SUB(0xFFFF000, 0xFF, Long));
    CHECK(false, CHECK_CARRY_SUB(0xFFFFFFFF, 1, Long));
    CHECK(false, CHECK_CARRY_SUB(0xFFFFFFFF, 0xFFFF, Long));
    CHECK(true, CHECK_CARRY_SUB(0, 0xFFFF, Long));
    CHECK(true, CHECK_CARRY_SUB(0xF0000000, 0xFFFFFFFF, Long));
    PASS();
}

TEST test_overflow_add()
{
    CHECK(false, CHECK_OVERFLOW_ADD(0, 0, Byte));
    CHECK(false, CHECK_OVERFLOW_ADD(0, 0x50, Byte));
    CHECK(false, CHECK_OVERFLOW_ADD(0x50, 0, Byte));
    CHECK(true, CHECK_OVERFLOW_ADD(0x50, 0x50, Byte));
    CHECK(true, CHECK_OVERFLOW_ADD(0x7F, 1, Byte));
    CHECK(false, CHECK_OVERFLOW_ADD(0x80, 0, Byte));
    CHECK(false, CHECK_OVERFLOW_ADD(0x80, 1, Byte));
    CHECK(false, CHECK_OVERFLOW_ADD(0x80, 0x50, Byte));
    CHECK(true, CHECK_OVERFLOW_ADD(0x80, 0x80, Byte));
    CHECK(false, CHECK_OVERFLOW_ADD(0xFF, 1, Byte));

    CHECK(false, CHECK_OVERFLOW_ADD(0, 0, Word));
    CHECK(false, CHECK_OVERFLOW_ADD(0x7F, 1, Word));
    CHECK(true, CHECK_OVERFLOW_ADD(0x7FFF, 1, Word));
    CHECK(false, CHECK_OVERFLOW_ADD(0xFFFF, 0, Word));
    CHECK(false, CHECK_OVERFLOW_ADD(0xFFFF, 1, Word));
    CHECK(true, CHECK_OVERFLOW_ADD(0x5000, 0x5000, Word));

    CHECK(false, CHECK_OVERFLOW_ADD(0, 0, Long));
    CHECK(false, CHECK_OVERFLOW_ADD(0x7F, 1, Long));
    CHECK(false, CHECK_OVERFLOW_ADD(0x7FFF, 1, Long));
    CHECK(true, CHECK_OVERFLOW_ADD(0x7FFFFFFF, 1, Long));
    CHECK(false, CHECK_OVERFLOW_ADD(0x7FFFFFFF, 0, Long));
    CHECK(false, CHECK_OVERFLOW_ADD(0xFFFFFFFF, 0, Long));
    CHECK(false, CHECK_OVERFLOW_ADD(0xFFFFFFFF, 1, Long));
    CHECK(true, CHECK_OVERFLOW_ADD(0x50000000, 0x50000000, Long));
    PASS();
}

TEST test_overflow_sub()
{
    CHECK(false, CHECK_OVERFLOW_SUB(0, 0, Byte));
    CHECK(false, CHECK_OVERFLOW_SUB(0, 0x50, Byte));
    CHECK(false, CHECK_OVERFLOW_SUB(0x50, 0, Byte));
    CHECK(false, CHECK_OVERFLOW_SUB(0x50, 0x50, Byte));
    CHECK(false, CHECK_OVERFLOW_SUB(0x7F, 1, Byte));
    CHECK(false, CHECK_OVERFLOW_SUB(0x80, 0, Byte));
    CHECK(true, CHECK_OVERFLOW_SUB(0x80, 1, Byte));
    CHECK(true, CHECK_OVERFLOW_SUB(0x80, 0x50, Byte));
    CHECK(false, CHECK_OVERFLOW_SUB(0x80, 0x80, Byte));
    CHECK(false, CHECK_OVERFLOW_SUB(0xFF, 1, Byte));

    CHECK(false, CHECK_OVERFLOW_SUB(0, 0, Word));
    CHECK(false, CHECK_OVERFLOW_SUB(0x7F, 1, Word));
    CHECK(false, CHECK_OVERFLOW_SUB(0x7FFF, 1, Word));
    CHECK(false, CHECK_OVERFLOW_SUB(0xFFFF, 0, Word));
    CHECK(false, CHECK_OVERFLOW_SUB(0xFFFF, 1, Word));
    CHECK(false, CHECK_OVERFLOW_SUB(0xFFFF, 0x8001, Word));
    CHECK(true, CHECK_OVERFLOW_SUB(0x1000, 0x8000, Word));
    CHECK(true, CHECK_OVERFLOW_SUB(0x8000, 0x2000, Word));

    CHECK(false, CHECK_OVERFLOW_SUB(0, 0, Long));
    CHECK(false, CHECK_OVERFLOW_SUB(0x7F, 1, Long));
    CHECK(false, CHECK_OVERFLOW_SUB(0x7FFF, 1, Long));
    CHECK(false, CHECK_OVERFLOW_SUB(0x7FFFFFFF, 0, Long));
    CHECK(false, CHECK_OVERFLOW_SUB(0x7FFFFFFF, 1, Long));
    CHECK(false, CHECK_OVERFLOW_SUB(0xFFFFFFFF, 0, Long));
    CHECK(false, CHECK_OVERFLOW_SUB(0xFFFFFFFF, 1, Long));
    CHECK(true, CHECK_OVERFLOW_SUB(0x80000000, 0x1, Long));
    CHECK(true, CHECK_OVERFLOW_SUB(0x80000000, 0x7FFFFFFF, Long));
    CHECK(true, CHECK_OVERFLOW_SUB(0x10000000, 0x90000000, Long));
    PASS();
}

TEST test_bin_parse()
{
    CHECK(0, bin_parse("0"));
    CHECK(0, bin_parse("000"));
    CHECK(0, bin_parse("0 0    0"));
    CHECK(0, bin_parse(""));
    CHECK(0, bin_parse("   "));

    CHECK(1, bin_parse("1"));
    CHECK(1, bin_parse("00 01"));

    CHECK(3, bin_parse("11"));
    CHECK(3, bin_parse("     1   1     "));

    CHECK(0xCC, bin_parse("11001100"));
    CHECK(0xE3, bin_parse("1110 0011"));
    CHECK(0xFF, bin_parse("1111 1111"));
    PASS();
}

TEST test_bin_tostring()
{
    char buffer[100];
    ASSERT_STR_EQ("0", bin_tostring(0, buffer));
    ASSERT_STR_EQ("1", bin_tostring(1, buffer));
    ASSERT_STR_EQ("10", bin_tostring(2, buffer));
    ASSERT_STR_EQ("11111111", bin_tostring(0xFF, buffer));
    ASSERT_STR_EQ("11111110", bin_tostring(0xFE, buffer));
    PASS();
}

SUITE(test_suite_bit_utils)
{
    RUN_TEST(test_bit);
    RUN_TEST(test_bit_set);
    RUN_TEST(test_bit_clr);
    RUN_TEST(test_bit_chg);

    RUN_TEST(test_fragment);

    RUN_TEST(test_mask_below);
    RUN_TEST(test_mask_below_inc);
    RUN_TEST(test_mask_above);
    RUN_TEST(test_mask_above_inc);

    RUN_TEST(test_sign_bit);

    RUN_TEST(test_carry_add);
    RUN_TEST(test_carry_sub);
    RUN_TEST(test_overflow_add);
    RUN_TEST(test_overflow_sub);

    RUN_TEST(test_bin_parse);
    RUN_TEST(test_bin_tostring);
}
