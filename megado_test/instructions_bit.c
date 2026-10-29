#include <greatest.h>
#include <m68k/m68k.h>

#include "globals.h"

TEST test_bchg_set(void)
{
    DATA(1, 2);
    DATA(5, 0xFFFF);
    RUN("0000 001 101 000101"); // BCHG D1, D5
    DATA_CHECK(5, 0xFFFB);
    PASS();
}

TEST test_bchg_cleared(void)
{
    DATA(1, 2);
    DATA(5, 0xFFFB);
    RUN("0000 001 101 000101"); // BCHG D1, D5
    DATA_CHECK(5, 0xFFFF);
    PASS();
}

TEST test_bclr(void)
{
    DATA(1, 2);
    DATA(0, 5);
    RUN("0000 001 110 000000"); // BCLR D1, D0
    DATA_CHECK(0, 1);
    PASS();
}

TEST test_bset(void)
{
    DATA(0, 2);
    DATA(5, 0);
    RUN("0000 000 111 000101"); // BSET D0, D5
    DATA_CHECK(5, 4);
    PASS();
}

TEST test_btst_true(void)
{
    DATA(6, 5);
    DATA(7, 0x1A0);
    RUN("0000 110 100 000111"); // BTST D6, D7
    CHECK(ZERO(m), 0);
    PASS();
}

TEST test_btst_false(void)
{
    DATA(6, 2);
    DATA(7, 0x1A0);
    RUN("0000 110 100 000111"); // BTST D6, D7
    CHECK(ZERO(m), 1);
    PASS();
}

SUITE(test_suite_instructions_bit)
{
    SET_SETUP(setup, NULL);
    SET_TEARDOWN(teardown, NULL);

    RUN_TEST(test_bchg_set);
    RUN_TEST(test_bchg_cleared);

    RUN_TEST(test_bclr);

    RUN_TEST(test_bset);

    RUN_TEST(test_btst_true);
    RUN_TEST(test_btst_false);
}
