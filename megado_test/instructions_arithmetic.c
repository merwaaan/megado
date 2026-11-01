#include <greatest.h>

#include "globals.h"

TEST test_clr_b()
{
    DATA(3, 0x1234ABCD);
    RUN("01000010 00 000011"); // CLR.b D3
    DATA_CHECK(3, 0x1234AB00);
    PASS();
}

TEST test_clr_w()
{
    DATA(3, 0x1234ABCD);
    RUN("01000010 01 000011"); // CLR.w D3
    DATA_CHECK(3, 0x12340000);
    PASS();
}

TEST test_clr_l()
{
    DATA(3, 0x1234ABCD);
    RUN("01000010 10 000011"); // CLR.l D3
    DATA_CHECK(3, 0);
    PASS();
}

TEST test_ext_b_to_w_zero()
{
    DATA(7, 0xAA00);
    RUN("0100100 010 000 111"); // EXT.w D7
    DATA_CHECK(7, 0);
    PASS();
}

TEST test_ext_b_to_w_pos()
{
    DATA(7, 0xAA0A);
    RUN("0100100 010 000 111"); // EXT.w D7
    DATA_CHECK(7, 0xA);
    PASS();
}

TEST test_ext_b_to_w_neg()
{
    DATA(7, 0xAC);
    RUN("0100100 010 000 111"); // EXT.w D7
    DATA_CHECK(7, 0xFFAC);
    PASS();
}

TEST test_ext_w_to_l_zero()
{
    DATA(7, 0xAABC0000);
    RUN("0100100 011 000 111"); // EXT.l D7
    DATA_CHECK(7, 0);
    PASS();
}

TEST test_ext_w_to_l_pos()
{
    DATA(7, 0xAABC0010);
    RUN("0100100 011 000 111"); // EXT.l D7
    DATA_CHECK(7, 0x10);
    PASS();
}

TEST test_ext_w_to_l_neg()
{
    DATA(7, 0x9DFC);
    RUN("0100100 011 000 111"); // EXT.l D7
    DATA_CHECK(7, 0xFFFF9DFC);
    PASS();
}

TEST test_mulu()
{
    DATA(0, 0xABCD0004);
    DATA(1, 0xFFFF0003);
    STATUS_SET(1, 1, 1, 1, 1);
    RUN("1100 001 011 000000"); // MULU D0, D1 (4 * 3)
    DATA_CHECK(1, 0x0000000C);
    STATUS_CHECK(1, 0, 0, 0, 0);
    PASS();
}

TEST test_mulu_max()
{
    DATA(0, 0xFFFF);
    DATA(1, 0xFFFF);
    STATUS_SET(1, 0, 1, 1, 1);
    RUN("1100 001 011 000000"); // MULU D0, D1 (0xFFFF * 0xFFFF)
    DATA_CHECK(1, 0xFFFE0001);
    STATUS_CHECK(1, 1, 0, 0, 0);
    PASS();
}

TEST test_mulu_zero()
{
    DATA(0, 0xABCD0000);
    DATA(1, 0x1234);
    STATUS_SET(1, 1, 0, 1, 1);
    RUN("1100 001 011 000000"); // MULU D0, D1 (0 * 0x1234)
    DATA_CHECK(1, 0);
    STATUS_CHECK(1, 0, 1, 0, 0);
    PASS();
}

TEST test_mulu_zero_low()
{
    DATA(0, 0x0100);
    DATA(1, 0x0100);
    STATUS_SET(1, 1, 1, 1, 1);
    RUN("1100 001 011 000000"); // MULU D0, D1 (0x100 * 0x100)
    DATA_CHECK(1, 0x00010000);
    STATUS_CHECK(1, 0, 0, 0, 0);
    PASS();
}

TEST test_mulu_imm()
{
    DATA(2, 0x00001234);
    STATUS_SET(1, 1, 1, 1, 1);
    MEM_W(2, 0x0010);
    RUN("1100 010 011 111100"); // MULU #0x0010, D2
    DATA_CHECK(2, 0x00012340);
    CHECK(4, m->pc);
    STATUS_CHECK(1, 0, 0, 0, 0);
    PASS();
}

TEST test_mulu_ind()
{
    ADDR(0, 0x1000);
    MEM_W(0x1000, 0x0200);
    DATA(3, 0x0300);
    STATUS_SET(1, 1, 1, 1, 1);
    RUN("1100 011 011 010000"); // MULU (A0), D3
    DATA_CHECK(3, 0x00060000);
    ADDR_CHECK(0, 0x1000);
    STATUS_CHECK(1, 0, 0, 0, 0);
    PASS();
}

TEST test_muls()
{
    DATA(0, 0x0003);
    DATA(1, 0x0004);
    STATUS_SET(1, 1, 1, 1, 1);
    RUN("1100 001 111 000000"); // MULS D0, D1 (3 * 4)
    DATA_CHECK(1, 0x0000000C);
    STATUS_CHECK(1, 0, 0, 0, 0);
    PASS();
}

TEST test_muls_neg_pos()
{
    DATA(0, 0xFFFE);
    DATA(1, 0x0003);
    STATUS_SET(1, 0, 1, 1, 1);
    RUN("1100 001 111 000000"); // MULS D0, D1 (-2 * 3)
    DATA_CHECK(1, 0xFFFFFFFA);
    STATUS_CHECK(1, 1, 0, 0, 0);
    PASS();
}

TEST test_muls_neg_neg()
{
    DATA(0, 0x8000);
    DATA(1, 0x8000);
    STATUS_SET(1, 1, 1, 1, 1);
    RUN("1100 001 111 000000"); // MULS D0, D1 (-32768 * -32768)
    DATA_CHECK(1, 0x40000000);
    STATUS_CHECK(1, 0, 0, 0, 0);
    PASS();
}

TEST test_muls_minus_one()
{
    DATA(0, 0xFFFF);
    DATA(1, 0x7FFF);
    STATUS_SET(1, 0, 1, 1, 1);
    RUN("1100 001 111 000000"); // MULS D0, D1 (-1 * 32767)
    DATA_CHECK(1, 0xFFFF8001);
    STATUS_CHECK(1, 1, 0, 0, 0);
    PASS();
}

TEST test_muls_upper_words_ignored()
{
    DATA(0, 0x1234FFFF);
    DATA(1, 0xABCD0002);
    STATUS_SET(1, 0, 1, 1, 1);
    RUN("1100 001 111 000000"); // MULS D0, D1 (-1 * 2)
    DATA_CHECK(1, 0xFFFFFFFE);
    STATUS_CHECK(1, 1, 0, 0, 0);
    PASS();
}

TEST test_muls_zero()
{
    DATA(0, 0xFFFF);
    DATA(1, 0xABCD0000);
    STATUS_SET(1, 1, 0, 1, 1);
    RUN("1100 001 111 000000"); // MULS D0, D1 (-1 * 0)
    DATA_CHECK(1, 0);
    STATUS_CHECK(1, 0, 1, 0, 0);
    PASS();
}

TEST test_muls_imm()
{
    DATA(2, 0x0010);
    STATUS_SET(1, 0, 1, 1, 1);
    MEM_W(2, 0xFFF0);
    RUN("1100 010 111 111100"); // MULS #-16, D2
    DATA_CHECK(2, 0xFFFFFF00);
    CHECK(4, m->pc);
    STATUS_CHECK(1, 1, 0, 0, 0);
    PASS();
}

SUITE(test_suite_instructions_arithmetic)
{
    SET_SETUP(setup, NULL);
    SET_TEARDOWN(teardown, NULL);

    RUN_TEST(test_clr_b);
    RUN_TEST(test_clr_w);
    RUN_TEST(test_clr_l);

    RUN_TEST(test_ext_b_to_w_zero);
    RUN_TEST(test_ext_b_to_w_pos);
    RUN_TEST(test_ext_b_to_w_neg);
    RUN_TEST(test_ext_w_to_l_zero);
    RUN_TEST(test_ext_w_to_l_pos);
    RUN_TEST(test_ext_w_to_l_neg);

    RUN_TEST(test_mulu);
    RUN_TEST(test_mulu_max);
    RUN_TEST(test_mulu_zero);
    RUN_TEST(test_mulu_zero_low);
    RUN_TEST(test_mulu_imm);
    RUN_TEST(test_mulu_ind);

    RUN_TEST(test_muls);
    RUN_TEST(test_muls_neg_pos);
    RUN_TEST(test_muls_neg_neg);
    RUN_TEST(test_muls_minus_one);
    RUN_TEST(test_muls_upper_words_ignored);
    RUN_TEST(test_muls_zero);
    RUN_TEST(test_muls_imm);
}
