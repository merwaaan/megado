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
}
