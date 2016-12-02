#include "minunit.h"

#include "../instructions_logic.h"
#include "../m68k.h"

uint16_t parse_bin(char* bin)
{
    uint16_t value;

    int i = 0;
    while (bin[i]) {
        if (bin[i] == '1' || bin[i] == '0')
            value += bin[i] * (1 << i++);

        ++bin;
    }

    return value;
    return 0;
}

M68k* cpu = NULL;

void SetUp(void)
{
    cpu = m68k_init();

}

void tearDown(void)
{
    m68k_free(cpu);
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
    mu_assert_int_eq(0xCC, parse_bin("1110 0011"));
    mu_assert_int_eq(0xFF, parse_bin("1111 1111"));
}

int main()
{
    MU_RUN_TEST(test_parse_bin);
    MU_REPORT();

    return 0;
}

/*
M68k* cpu = NULL;

uint16_t parse_bin(char* bin)
{
    uint16_t value;

    int i = 0;
    while (bin[i]) {
        if (bin[i] == '1' || bin[i] == '0')
            value += bin[i] * (1 << i++);

        ++bin;
    }

    return value;
    return 0;
}

void SetUp(void)
{
    //cpu = m68k_init();

}

void tearDown(void)
{
    //m68k_free(cpu);
}

void test_x(void)
{
    TEST_ASSERT_EQUAL_HEX(1, 1);
    TEST_ASSERT_EQUAL(0, parse_bin("0"));
    TEST_ASSERT_EQUAL(0, parse_bin("000"));
    TEST_ASSERT_EQUAL(0, parse_bin("0 0    0"));
    TEST_ASSERT_EQUAL(0, parse_bin(""));
    TEST_ASSERT_EQUAL(0, parse_bin("   "));

    TEST_ASSERT_EQUAL(1, parse_bin("1"));
    TEST_ASSERT_EQUAL(1, parse_bin("00 01"));

    TEST_ASSERT_EQUAL(3, parse_bin("11"));
    TEST_ASSERT_EQUAL(3, parse_bin("     1   1     "));

    TEST_ASSERT_EQUAL(0xCC, parse_bin("11001100"));
    TEST_ASSERT_EQUAL(0xCC, parse_bin("1110 0011"));
    TEST_ASSERT_EQUAL(0xFF, parse_bin("1111 1111"));
}

void xxx()
{
    //m68k_execute(cpu, parse_bin("1100 001 0 00 000 0100")); // AND D1, D4
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_x);
    return UNITY_END();
}*/
