#pragma once

#include <minunit.h>
#include <m68k/m68k.h>

#include "globals.h"

MU_TEST(test_exg_data_data)
{
    DATA(3, 0x12345678); DATA(6, 0xA0A0A0A0);
    RUN("1100 011 1 01000 110"); // EXG D3, D6
    DATA_CHECK(3, 0xA0A0A0A0); DATA_CHECK(6, 0x12345678);
}

MU_TEST(test_exg_address_address)
{
    ADDR(0, 0x12345678); ADDR(2, 0xA0A0A0A0);
    RUN("1100 000 1 01001 010"); // EXG A0, A2
    ADDR_CHECK(0, 0xA0A0A0A0); ADDR_CHECK(2, 0x12345678);
}

MU_TEST(test_exg_data_address)
{
    DATA(4, 0x12345678); ADDR(0, 0xA0A0A0A0);
    RUN("1100 100 1 10001 000"); // EXG D4, A0
    DATA_CHECK(4, 0xA0A0A0A0); ADDR_CHECK(0, 0x12345678);
}

MU_TEST(test_lea)
{
    DATA(1, 0xF0F0); ADDR(0, 0xFFFF0000);
    RUN("0100 000 111 000001"); // LEA D1, A1
    ADDR_CHECK(0, 0xF0F0);
}

MU_TEST(test_movem) // TODO
{
    ADDR(5, 0x294);
    MEM16(0x294, 0x8000);
    MEM16(0x294, 0x3FFF);
    MEM16(0x294, 0x0100);
    RUN("01001 1 001 0 011101"); // MOVEM.w (A5)+, D5-D7
    // TODO mask
    DATA_CHECK(5, 0xFFFF8000);
    DATA_CHECK(6, 0x00003FFF);
    DATA_CHECK(7, 0x00000100);
}

MU_TEST_SUITE(test_suite_instructions_transfer)
{
    MU_SUITE_CONFIGURE(&setup, &teardown);

    //MU_RUN_TEST(test_exg_data_data); //TODO fix exg conflict with add
    //MU_RUN_TEST(test_exg_address_address);
    //MU_RUN_TEST(test_exg_data_address);

    MU_RUN_TEST(test_lea);

    MU_RUN_TEST(test_movem);
}
