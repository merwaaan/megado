#pragma once

#include "minunit.h"

#include "../bit_utils.h"

MU_TEST(test_parse_bin)
{
    mu_assert_int_eq(0, parse_bin("0\0"));
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
