#include <greatest.h>

#include "globals.h"

TEST test_exg_data_data()
{
    DATA(3, 0x12345678);
    DATA(6, 0xA0A0A0A0);
    RUN("1100 011 1 01000 110"); // EXG D3, D6
    DATA_CHECK(3, 0xA0A0A0A0);
    DATA_CHECK(6, 0x12345678);
    PASS();
}

TEST test_exg_address_address()
{
    ADDR(0, 0x12345678);
    ADDR(2, 0xA0A0A0A0);
    RUN("1100 000 1 01001 010"); // EXG A0, A2
    ADDR_CHECK(0, 0xA0A0A0A0);
    ADDR_CHECK(2, 0x12345678);
    PASS();
}

TEST test_exg_data_address()
{
    DATA(4, 0x12345678);
    ADDR(0, 0xA0A0A0A0);
    RUN("1100 100 1 10001 000"); // EXG D4, A0
    DATA_CHECK(4, 0xA0A0A0A0);
    ADDR_CHECK(0, 0x12345678);
    PASS();
}

TEST test_lea()
{
    ADDR(1, 0xF0F0);
    ADDR(0, 0xFFFF0000);
    RUN("0100 000 111 010001"); // LEA (A1), A0
    ADDR_CHECK(0, 0xF0F0);
    PASS();
}

TEST test_move()
{
    SKIPm("not implemented");
}

TEST test_movem_mem_to_reg()
{
    ADDR(5, 0x294);
    MEM_W(0x294, 0x8000);
    MEM_W(0x296, 0x3FFF);
    MEM_W(0x298, 0x0100);
    MEM_W(2, 0x00E0);            // Mask after the opcode in memory
    RUN("01001 1 001 0 011101"); // MOVEM.w (A5)+, D5-D7
                                 // TODO mask
    DATA_CHECK(5, 0xFFFF8000);
    DATA_CHECK(6, 0x00003FFF);
    DATA_CHECK(7, 0x00000100);
    PASS();
}

SUITE(test_suite_instructions_transfer)
{
    SET_SETUP(setup, NULL);
    SET_TEARDOWN(teardown, NULL);

    RUN_TEST(test_exg_data_data);
    RUN_TEST(test_exg_address_address);
    RUN_TEST(test_exg_data_address);

    RUN_TEST(test_lea);

    RUN_TEST(test_move);
    RUN_TEST(test_movem_mem_to_reg);
}
