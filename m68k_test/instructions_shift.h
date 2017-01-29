#pragma once

#include <minunit.h>
#include <m68k/m68k.h>

#include "globals.h"

MU_TEST(test_lsl)
{
    DATA(7, 1);

    DATA(0, 0xFFFFFFFF);
    RUN("1110 111 1 00 1 01 000"); // LSL.b D7, D0 (1 bit)
    DATA_CHECK(0, 0xFFFFFFFE);

    DATA(0, 0xFFFFFFFF);
    RUN("1110 111 1 01 1 01 000"); // LSL.w D7, D0 (1 bit)
    DATA_CHECK(0, 0xFFFFFFFE);

    DATA(0, 0xFFFFFFFF);
    RUN("1110 111 1 10 1 01 000"); // LSL.l D7, D0 (1 bit)
    DATA_CHECK(0, 0xFFFFFFFE);

    DATA(7, 10);

    DATA(0, 0xFFFFFFFF);
    RUN("1110 111 1 00 1 01 000"); // LSL.b D7, D0 (10 bits)
    DATA_CHECK(0, 0xFFFFFF00);

    DATA(0, 0xFFFFFFFF);
    RUN("1110 111 1 01 1 01 000"); // LSL.w D7, D0 (10 bits)
    DATA_CHECK(0, 0xFFFFFC00);

    DATA(0, 0xFFFFFFFF);
    RUN("1110 111 1 10 1 01 000"); // LSL.l D7, D0 (10 bits)
    DATA_CHECK(0, 0xFFFFFC00);
}

MU_TEST(test_lsr)
{
    DATA(7, 1);

    DATA(0, 0xFFFFFFFF);
    RUN("1110 111 0 00 1 01 000"); // LSR.b D7, D0 (1 bit)
    DATA_CHECK(0, 0xFFFFFF7F);

    DATA(0, 0xFFFFFFFF);
    RUN("1110 111 0 01 1 01 000"); // LSR.w D7, D0 (1 bit)
    DATA_CHECK(0, 0xFFFF7FFF);

    DATA(0, 0xFFFFFFFF);
    RUN("1110 111 0 10 1 01 000"); // LSR.l D7, D0 (1 bit)
    DATA_CHECK(0, 0x7FFFFFFF);

    DATA(7, 10);

    DATA(0, 0xFFFFFFFF);
    RUN("1110 111 0 00 1 01 000"); // LSR.b D7, D0 (10 bits)
    DATA_CHECK(0, 0xFFFFFF00);

    DATA(0, 0xFFFFFFFF);
    RUN("1110 111 0 01 1 01 000"); // LSR.w D7, D0 (10 bits)
    DATA_CHECK(0, 0xFFFF003F);

    DATA(0, 0xFFFFFFFF);
    RUN("1110 111 0 10 1 01 000"); // LSR.l D7, D0 (10 bits)
    DATA_CHECK(0, 0x3FFFFF);
}

MU_TEST(test_asl)
{
    DATA(2, 5);

    DATA(1, 0xF0F0F0F0);
    RUN("1110 010 1 00 1 00 001"); // ASL.b D2, D1 (5 bits)
    DATA_CHECK(1, 0xF0F0F000);

    DATA(1, 0xF0F0F0F0);
    RUN("1110 010 1 01 1 00 001"); // ASL.w D2, D1 (5 bits)
    DATA_CHECK(1, 0xF0F01E00);

    DATA(1, 0xF0F0F0F0);
    RUN("1110 010 1 10 1 00 001"); // ASL.l D2, D1 (5 bits)
    DATA_CHECK(1, 0x1E1E1E00);

    DATA(2, 20);

    DATA(1, 0xF0F0F0F0);
    RUN("1110 010 1 00 1 00 001"); // ASL.b D2, D1 (20 bits)
    DATA_CHECK(1, 0xF0F0F000);

    DATA(1, 0xF0F0F0F0);
    RUN("1110 010 1 01 1 00 001"); // ASL.w D2, D1 (20 bits)
    DATA_CHECK(1, 0xF0F00000);

    DATA(1, 0xF0F0F0F0);
    RUN("1110 010 1 10 1 00 001"); // ASL.l D2, D1 (20 bits)
    DATA_CHECK(1, 0x0F000000);
}

MU_TEST(test_asr)
{
    DATA(2, 3);

    DATA(1, 0xF0F0F0F0);
    RUN("1110 010 0 00 1 00 001"); // ASR.b D2, D1 (3 bits)
    DATA_CHECK(1, 0xF0F0F0FE);

    DATA(1, 0xF0F0F0F0);
    RUN("1110 010 0 01 1 00 001"); // ASR.w D2, D1 (3 bits)
    DATA_CHECK(1, 0xF0F0FE1E);

    DATA(1, 0xF0F0F0F0);
    RUN("1110 010 0 10 1 00 001"); // ASR.l D2, D1 (3 bits)
    DATA_CHECK(1, 0xFE1E1E1E);

    DATA(1, 0x0F0F0F0F);
    RUN("1110 010 0 00 1 00 001"); // ASR.b D2, D1 (3 bits)
    DATA_CHECK(1, 0x0F0F0F01);

    DATA(1, 0x0F0F0F0F);
    RUN("1110 010 0 01 1 00 001"); // ASR.w D2, D1 (3 bits)
    DATA_CHECK(1, 0x0F0F01E1);

    DATA(1, 0x0F0F0F0F);
    RUN("1110 010 0 10 1 00 001"); // ASR.l D2, D1 (3 bits)
    DATA_CHECK(1, 0x01E1E1E1);
}

MU_TEST(test_rol)
{
    DATA(0, 1);

    DATA(2, 0xB38F0F83);
    RUN("1110 000 1 00 1 11 010"); // ROL.b D0, D2 (1 bit)
    DATA_CHECK(2, 0xB38F0F07);

    DATA(2, 0xB38F0F83);
    RUN("1110 000 1 01 1 11 010"); // ROL.w D0, D2 (1 bit)
    DATA_CHECK(2, 0xB38F1F06);

    DATA(2, 0xB38F0F83);
    RUN("1110 000 1 10 1 11 010"); // ROL.l D0, D2 (1 bit)
    DATA_CHECK(2, 0x671E1F07);

    DATA(0, 10);

    DATA(2, 0xB38F0F83);
    RUN("1110 000 1 00 1 11 010"); // ROL.b D0, D2 (10 bits)
    DATA_CHECK(2, 0xB38F0F0E);

    DATA(2, 0xB38F0F83);
    RUN("1110 000 1 01 1 11 010"); // ROL.w D0, D2 (10 bits)
    DATA_CHECK(2, 0xB38F0C3E);

    DATA(2, 0xB38F0F83);
    RUN("1110 000 1 10 1 11 010"); // ROL.l D0, D2 (10 bits)
    DATA_CHECK(2, 0x3C3E0ECE);
}

MU_TEST(test_swap)
{
    DATA(4, 0x1234ABCD);
    RUN("0100100001000 100"); // SWAP D4
    DATA_CHECK(4, 0xABCD1234);
}

MU_TEST_SUITE(test_suite_instructions_shift)
{
    MU_SUITE_CONFIGURE(&setup, &teardown);

    MU_RUN_TEST(test_lsl);
    MU_RUN_TEST(test_lsr);

    MU_RUN_TEST(test_asl);
    MU_RUN_TEST(test_asr);

    MU_RUN_TEST(test_rol);

    MU_RUN_TEST(test_swap);
}
