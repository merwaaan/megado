#include <greatest.h>
#include <m68k/m68k.h>
#include <stdbool.h>

#include "globals.h"

TEST test_and_b(void)
{
    DATA(2, 0xFE800174);
    DATA(3, 0x25CFB7DD);
    RUN("1100 011 0 00 000010"); // AND.b D2, D3
    DATA_CHECK(3, 0x25CFB754);
    PASS();
}

TEST test_and_w(void)
{
    DATA(2, 0xFE800174);
    DATA(3, 0x25CFB7DD);
    RUN("1100 011 0 01 000010"); // AND.w D2, D3
    DATA_CHECK(3, 0x25CF0154);
    PASS();
}

TEST test_and_l(void)
{
    DATA(2, 0xFE800174);
    DATA(3, 0x25CFB7DD);
    RUN("1100 011 0 10 000010"); // AND.l D2, D3
    DATA_CHECK(3, 0x24800154);
    PASS();
}

TEST test_eor_b(void)
{
    DATA(4, 0x25CFB7DD);
    DATA(1, 0xFE800174);
    RUN("1011 100 1 00 000001"); // EOR.b D4, D1
    DATA_CHECK(1, 0xFE8001A9);
    PASS();
}

TEST test_eor_w(void)
{
    DATA(4, 0x25CFB7DD);
    DATA(1, 0xFE800174);
    RUN("1011 100 1 01 000001"); // EOR.w D4, D1
    DATA_CHECK(1, 0xFE80B6A9);
    PASS();
}

TEST test_eor_l(void)
{
    DATA(4, 0x25CFB7DD);
    DATA(1, 0xFE800174);
    RUN("1011 100 1 10 000001"); // EOR.l D4, D1
    DATA_CHECK(1, 0xDB4FB6A9);
    PASS();
}

TEST test_or_b(void)
{
    DATA(4, 0x25CFB7DD);
    DATA(1, 0xFE800174);
    RUN("1000 001 0 00 000100"); // OR.b D4, D1
    DATA_CHECK(1, 0xFE8001FD);
    PASS();
}

TEST test_or_w(void)
{
    DATA(4, 0x25CFB7DD);
    DATA(1, 0xFE800174);
    RUN("1000 001 0 01 000100"); // OR.w D4, D1
    DATA_CHECK(1, 0xFE80B7FD);
    PASS();
}

TEST test_or_l(void)
{
    DATA(4, 0x25CFB7DD);
    DATA(1, 0xFE800174);
    RUN("1000 001 0 10 000100"); // OR.l D4, D1
    DATA_CHECK(1, 0xFFCFB7FD);
    PASS();
}

TEST test_andi_b(void) // TODO
{
    DATA(0, 0xF0F0F0F0);
    // MEM()
    RUN("00000010 00 000000"); // ANDI.b #FF D2
    DATA_CHECK(0, 0);          // TODO
    PASS();
}

// TODO ori, eori

TEST test_not_b(void)
{
    DATA(7, 0xFE800174);
    RUN("01000110 00 000111"); // NOT.b D7
    DATA_CHECK(7, 0xFE80018B);
    PASS();
}

TEST test_not_w(void)
{
    DATA(7, 0xFE800174);
    RUN("01000110 01 000111"); // NOT.w D7
    DATA_CHECK(7, 0xFE80FE8B);
    PASS();
}

TEST test_not_l(void)
{
    DATA(7, 0xFE800174);
    RUN("01000110 10 000111"); // NOT.l D7
    DATA_CHECK(7, 0x017FFE8B);
    PASS();
}

TEST test_tst_b_true(void)
{
    DATA(4, 0xFE800100);
    RUN("01001010 00 000100"); // TST.b D4
    CHECK(true, ZERO(m));
    PASS();
}

TEST test_tst_b_false(void)
{
    DATA(4, 0xFE80010C);
    RUN("01001010 00 000100"); // TST.b D4
    CHECK(false, ZERO(m));
    PASS();
}

TEST test_tst_w_true(void)
{
    DATA(4, 0xFAAF0000);
    RUN("01001010 01 000100"); // TST.w D4
    CHECK(true, ZERO(m));
    PASS();
}

TEST test_tst_w_false(void)
{
    DATA(4, 0xFAAF0FF0);
    RUN("01001010 01 000100"); // TST.w D4
    CHECK(false, ZERO(m));
    PASS();
}

TEST test_tst_l_true(void)
{
    DATA(4, 0);
    RUN("01001010 10 000100"); // TST.l D4
    CHECK(true, ZERO(m));
    PASS();
}

TEST test_tst_l_false(void)
{
    DATA(4, 0xF0000000);
    RUN("01001010 10 000100"); // TST.l D4
    CHECK(false, ZERO(m));
    PASS();
}

// TODO or, eor

SUITE(test_suite_instructions_logic)
{
    SET_SETUP(setup, NULL);
    SET_TEARDOWN(teardown, NULL);

    RUN_TEST(test_and_b);
    RUN_TEST(test_and_w);
    RUN_TEST(test_and_l);

    RUN_TEST(test_eor_b);
    RUN_TEST(test_eor_w);
    RUN_TEST(test_eor_l);

    RUN_TEST(test_or_b);
    RUN_TEST(test_or_w);
    RUN_TEST(test_or_l);

    RUN_TEST(test_not_b);
    RUN_TEST(test_not_w);
    RUN_TEST(test_not_l);

    RUN_TEST(test_tst_b_true);
    RUN_TEST(test_tst_b_false);
    RUN_TEST(test_tst_w_true);
    RUN_TEST(test_tst_w_false);
    RUN_TEST(test_tst_l_true);
    RUN_TEST(test_tst_l_false);
}
