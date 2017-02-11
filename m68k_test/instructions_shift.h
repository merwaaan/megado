#pragma once

#include <minunit.h>
#include <m68k/m68k.h>

#include "globals.h"

MU_TEST(test_lsl) // refactor with asl
{
    DATA(7, 1);

    DATA(0, 0x12345678);
    RUN("1110 111 1 00 1 01 000"); // LSL.b D7, D0 (1 bit)
    DATA_CHECK(0, 0x123456F0);
    STATUS_CHECK(0, 1, 0, 0, 0);

    DATA(0, 0x12345678);
    RUN("1110 111 1 01 1 01 000"); // LSL.w D7, D0 (1 bit)
    DATA_CHECK(0, 0x1234ACF0);
    STATUS_CHECK(0, 1, 0, 0, 0);

    DATA(0, 0x12345678);
    RUN("1110 111 1 10 1 01 000"); // LSL.l D7, D0 (1 bit)
    DATA_CHECK(0, 0x2468ACF0);
    STATUS_CHECK(0, 0, 0, 0, 0);

    DATA(7, 10);

    DATA(0, 0x12345678);
    RUN("1110 111 1 00 1 01 000"); // LSL.b D7, D0 (10 bits)
    DATA_CHECK(0, 0x12345600);
    STATUS_CHECK(0, 0, 1, 0, 0);

    DATA(0, 0x12345678);
    RUN("1110 111 1 01 1 01 000"); // LSL.w D7, D0 (10 bits)
    DATA_CHECK(0, 0x1234E000);
    STATUS_CHECK(1, 1, 0, 0, 1);

    DATA(0, 0x12345678);
    RUN("1110 111 1 10 1 01 000"); // LSL.l D7, D0 (10 bits)
    DATA_CHECK(0, 0xD159E000);
    STATUS_CHECK(0, 1, 0, 0, 0);

    DATA(2, 5);

    DATA(1, 0xF0F0F0F0);
    RUN("1110 010 1 00 1 01 001"); // LSL.b D2, D1 (5 bits)
    DATA_CHECK(1, 0xF0F0F000);
    STATUS_CHECK(0, 0, 1, 0, 0);

    DATA(1, 0xF0F0F0F0);
    RUN("1110 010 1 01 1 01 001"); // LSL.w D2, D1 (5 bits)
    DATA_CHECK(1, 0xF0F01E00);
    STATUS_CHECK(0, 0, 0, 0, 0);

    DATA(1, 0xF0F0F0F0);
    RUN("1110 010 1 10 1 01 001"); // LSL.l D2, D1 (5 bits)
    DATA_CHECK(1, 0x1E1E1E00);
    STATUS_CHECK(0, 0, 0, 0, 0);

    DATA(2, 20);

    DATA(1, 0xF0F0F0F0);
    RUN("1110 010 1 00 1 01 001"); // LSL.b D2, D1 (20 bits)
    DATA_CHECK(1, 0xF0F0F000);
    STATUS_CHECK(0, 0, 1, 0, 0);

    DATA(1, 0xF0F0F0F0);
    RUN("1110 010 1 01 1 01 001"); // LSL.w D2, D1 (20 bits)
    DATA_CHECK(1, 0xF0F00000);
    STATUS_CHECK(0, 0, 1, 0, 0);

    DATA(1, 0xF0F0F0F0);
    RUN("1110 010 1 10 1 01 001"); // LSL.l D2, D1 (20 bits)
    DATA_CHECK(1, 0x0F000000);
    STATUS_CHECK(1, 0, 0, 0, 1);
}

MU_TEST(test_lsr)
{
    DATA(7, 1);

    DATA(0, 0x12345601);
    RUN("1110 111 0 00 1 01 000"); // LSR.b D7, D0 (1 bit)
    DATA_CHECK(0, 0x12345600);
    STATUS_CHECK(1, 0, 1, 0, 1);

    DATA(0, 0x12345678);
    RUN("1110 111 0 00 1 01 000"); // LSR.b D7, D0 (1 bit)
    DATA_CHECK(0, 0x1234563C);
    STATUS_CHECK(0, 0, 0, 0, 0);

    DATA(0, 0x12345678);
    RUN("1110 111 0 01 1 01 000"); // LSR.w D7, D0 (1 bit)
    DATA_CHECK(0, 0x12342B3C);
    STATUS_CHECK(0, 0, 0, 0, 0);

    DATA(0, 0x12345678);
    RUN("1110 111 0 10 1 01 000"); // LSR.l D7, D0 (1 bit)
    DATA_CHECK(0, 0x091A2B3C);
    STATUS_CHECK(0, 0, 0, 0, 0);

    DATA(7, 10);

    DATA(0, 0x12345678);
    RUN("1110 111 0 00 1 01 000"); // LSR.b D7, D0 (10 bits)
    DATA_CHECK(0, 0x12345600);
    STATUS_CHECK(0, 0, 1, 0, 0);

    DATA(0, 0x12345678);
    RUN("1110 111 0 01 1 01 000"); // LSR.w D7, D0 (10 bits)
    DATA_CHECK(0, 0x12340015);
    STATUS_CHECK(1, 0, 0, 0, 1);

    DATA(0, 0x12345678);
    RUN("1110 111 0 10 1 01 000"); // LSR.l D7, D0 (10 bits)
    DATA_CHECK(0, 0x00048D15);
    STATUS_CHECK(1, 0, 0, 0, 1);
}

MU_TEST(test_asl)
{
    DATA(7, 1);

    DATA(0, 0x12345678);
    RUN("1110 111 1 00 1 00 000"); // ASL.b D7, D0 (1 bit)
    DATA_CHECK(0, 0x123456F0);
    STATUS_CHECK(0, 1, 0, 0, 0);

    DATA(0, 0x12345678);
    RUN("1110 111 1 01 1 00 000"); // ASL.w D7, D0 (1 bit)
    DATA_CHECK(0, 0x1234ACF0);
    STATUS_CHECK(0, 1, 0, 0, 0);

    DATA(0, 0x12345678);
    RUN("1110 111 1 10 1 00 000"); // ASL.l D7, D0 (1 bit)
    DATA_CHECK(0, 0x2468ACF0);
    STATUS_CHECK(0, 0, 0, 0, 0);

    DATA(7, 10);

    DATA(0, 0x12345678);
    RUN("1110 111 1 00 1 00 000"); // ASL.b D7, D0 (10 bits)
    DATA_CHECK(0, 0x12345600);
    STATUS_CHECK(0, 0, 1, 0, 0);

    DATA(0, 0x12345678);
    RUN("1110 111 1 01 1 00 000"); // ASL.w D7, D0 (10 bits)
    DATA_CHECK(0, 0x1234E000);
    STATUS_CHECK(1, 1, 0, 0, 1);

    DATA(0, 0x12345678);
    RUN("1110 111 1 10 1 00 000"); // ASL.l D7, D0 (10 bits)
    DATA_CHECK(0, 0xD159E000);
    STATUS_CHECK(0, 1, 0, 0, 0);

    DATA(2, 5);

    DATA(1, 0xF0F0F0F0);
    RUN("1110 010 1 00 1 00 001"); // ASL.b D2, D1 (5 bits)
    DATA_CHECK(1, 0xF0F0F000);
    STATUS_CHECK(0, 0, 1, 0, 0);

    DATA(1, 0xF0F0F0F0);
    RUN("1110 010 1 01 1 00 001"); // ASL.w D2, D1 (5 bits)
    DATA_CHECK(1, 0xF0F01E00);
    STATUS_CHECK(0, 0, 0, 0, 0);

    DATA(1, 0xF0F0F0F0);
    RUN("1110 010 1 10 1 00 001"); // ASL.l D2, D1 (5 bits)
    DATA_CHECK(1, 0x1E1E1E00);
    STATUS_CHECK(0, 0, 0, 0, 0);

    DATA(2, 20);

    DATA(1, 0xF0F0F0F0);
    RUN("1110 010 1 00 1 00 001"); // ASL.b D2, D1 (20 bits)
    DATA_CHECK(1, 0xF0F0F000);
    STATUS_CHECK(0, 0, 1, 0, 0);

    DATA(1, 0xF0F0F0F0);
    RUN("1110 010 1 01 1 00 001"); // ASL.w D2, D1 (20 bits)
    DATA_CHECK(1, 0xF0F00000);
    STATUS_CHECK(0, 0, 1, 0, 0);

    DATA(1, 0xF0F0F0F0);
    RUN("1110 010 1 10 1 00 001"); // ASL.l D2, D1 (20 bits)
    DATA_CHECK(1, 0x0F000000);
    STATUS_CHECK(1, 0, 0, 0, 1);
}

MU_TEST(test_asr)
{
    DATA(2, 3);

    DATA(1, 0xF0F0F0F0);
    RUN("1110 010 0 00 1 00 001"); // ASR.b D2, D1 (3 bits)
    DATA_CHECK(1, 0xF0F0F0FE);
    STATUS_CHECK(0, 1, 0, 0, 0);

    DATA(1, 0xF0F0F0F0);
    RUN("1110 010 0 01 1 00 001"); // ASR.w D2, D1 (3 bits)
    DATA_CHECK(1, 0xF0F0FE1E);
    STATUS_CHECK(0, 1, 0, 0, 0);

    DATA(1, 0xF0F0F0F0);
    RUN("1110 010 0 10 1 00 001"); // ASR.l D2, D1 (3 bits)
    DATA_CHECK(1, 0xFE1E1E1E);
    STATUS_CHECK(0, 1, 0, 0, 0);

    DATA(1, 0x0F0F0F0F);
    RUN("1110 010 0 00 1 00 001"); // ASR.b D2, D1 (3 bits)
    DATA_CHECK(1, 0x0F0F0F01);
    STATUS_CHECK(1, 0, 0, 0, 1);

    DATA(1, 0x0F0F0F0F);
    RUN("1110 010 0 01 1 00 001"); // ASR.w D2, D1 (3 bits)
    DATA_CHECK(1, 0x0F0F01E1);
    STATUS_CHECK(1, 0, 0, 0, 1);

    DATA(1, 0x0F0F0F0F);
    RUN("1110 010 0 10 1 00 001"); // ASR.l D2, D1 (3 bits)
    DATA_CHECK(1, 0x01E1E1E1);
    STATUS_CHECK(1, 0, 0, 0, 1);

    DATA(7, 1);

    DATA(0, 0x12345601);
    RUN("1110 111 0 00 1 00 000"); // ASR.b D7, D0 (1 bit)
    DATA_CHECK(0, 0x12345600);
    STATUS_CHECK(1, 0, 1, 0, 1);

    DATA(0, 0x12345680);
    RUN("1110 111 0 00 1 00 000"); // ASR.b D7, D0 (1 bit)
    DATA_CHECK(0, 0x123456C0);
    STATUS_CHECK(0, 1, 0, 0, 0);

    DATA(0, 0x12345678);
    RUN("1110 111 0 00 1 00 000"); // ASR.b D7, D0 (1 bit)
    DATA_CHECK(0, 0x1234563C);
    STATUS_CHECK(0, 0, 0, 0, 0);

    DATA(0, 0x12345678);
    RUN("1110 111 0 01 1 00 000"); // ASR.w D7, D0 (1 bit)
    DATA_CHECK(0, 0x12342B3C);
    STATUS_CHECK(0, 0, 0, 0, 0);

    DATA(0, 0x12345678);
    RUN("1110 111 0 10 1 00 000"); // ASR.l D7, D0 (1 bit)
    DATA_CHECK(0, 0x091A2B3C);
    STATUS_CHECK(0, 0, 0, 0, 0);

    DATA(7, 10);

    DATA(0, 0x12345678);
    RUN("1110 111 0 00 1 00 000"); // ASR.b D7, D0 (10 bits)
    DATA_CHECK(0, 0x12345600);
    STATUS_CHECK(0, 0, 1, 0, 0);

    DATA(0, 0x12345678);
    RUN("1110 111 0 01 1 00 000"); // ASR.w D7, D0 (10 bits)
    DATA_CHECK(0, 0x12340015);
    STATUS_CHECK(1, 0, 0, 0, 1);

    DATA(0, 0x12345678);
    RUN("1110 111 0 10 1 00 000"); // ASR.l D7, D0 (10 bits)
    DATA_CHECK(0, 0x00048D15);
    STATUS_CHECK(1, 0, 0, 0, 1);
}

MU_TEST(test_rol)
{
    DATA(6, 1);

    DATA(1, 0x12345678);
    RUN("1110 110 1 00 1 11 001"); // ROL.b D6, D1 (1 bit)
    DATA_CHECK(1, 0x123456F0);
    STATUS_CHECK(0, 1, 0, 0, 0);

    DATA(1, 0x12345687);
    RUN("1110 110 1 00 1 11 001"); // ROL.b D6, D1 (1 bit)
    DATA_CHECK(1, 0x1234560F);
    STATUS_CHECK(0, 0, 0, 0, 1);

    DATA(1, 0x12345678);
    RUN("1110 110 1 01 1 11 001"); // ROL.w D6, D1 (1 bit)
    DATA_CHECK(1, 0x1234ACF0);
    STATUS_CHECK(0, 1, 0, 0, 0);

    DATA(1, 0x1234D687);
    RUN("1110 110 1 01 1 11 001"); // ROL.w D6, D1 (1 bit)
    DATA_CHECK(1, 0x1234AD0F);
    STATUS_CHECK(0, 1, 0, 0, 1);

    DATA(1, 0x12345678);
    RUN("1110 110 1 10 1 11 001"); // ROL.l D6, D1 (1 bit)
    DATA_CHECK(1, 0x2468ACF0);
    STATUS_CHECK(0, 0, 0, 0, 0);

    DATA(1, 0x92345678);
    RUN("1110 110 1 10 1 11 001"); // ROL.l D6, D1 (1 bit)
    DATA_CHECK(1, 0x2468ACF1);
    STATUS_CHECK(0, 0, 0, 0, 1);

    DATA(6, 12);

    DATA(1, 0x12345678);
    RUN("1110 110 1 00 1 11 001"); // ROL.b D6, D1 (12 bits)
    DATA_CHECK(1, 0x12345687);
    STATUS_CHECK(0, 1, 0, 0, 1);

    DATA(1, 0x12345687);
    RUN("1110 110 1 00 1 11 001"); // ROL.b D6, D1 (12 bits)
    DATA_CHECK(1, 0x12345678);
    STATUS_CHECK(0, 0, 0, 0, 0);

    DATA(1, 0x12345678);
    RUN("1110 110 1 01 1 11 001"); // ROL.w D6, D1 (12 bits)
    DATA_CHECK(1, 0x12348567);
    STATUS_CHECK(0, 1, 0, 0, 1);

    DATA(1, 0x1234D687);
    RUN("1110 110 1 01 1 11 001"); // ROL.w D6, D1 (12 bits)
    DATA_CHECK(1, 0x12347D68);
    STATUS_CHECK(0, 0, 0, 0, 0);

    DATA(1, 0x12345678);
    RUN("1110 110 1 10 1 11 001"); // ROL.l D6, D1 (12 bits)
    DATA_CHECK(1, 0x45678123);
    STATUS_CHECK(0, 0, 0, 0, 1);

    DATA(1, 0x92345678);
    RUN("1110 110 1 10 1 11 001"); // ROL.l D6, D1 (12 bits)
    DATA_CHECK(1, 0x45678923);
    STATUS_CHECK(0, 0, 0, 0, 1);
}

MU_TEST(test_ror) // TODO
{
}

MU_TEST(test_roxl)
{
}

MU_TEST(test_roxr)
{
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
    MU_RUN_TEST(test_ror);

    MU_RUN_TEST(test_roxl);
    MU_RUN_TEST(test_roxr);

    MU_RUN_TEST(test_swap);
}
