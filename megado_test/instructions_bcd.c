#include <greatest.h>

#include "globals.h"

// N and V are undefined after BCD instructions so only X, Z and C are checked

#define BCD_STATUS_CHECK(x, z, c) \
    CHECK(x, EXTENDED(m));        \
    CHECK(z, ZERO(m));            \
    CHECK(c, CARRY(m))

TEST test_abcd_basic()
{
    DATA(0, 0x12);
    DATA(1, 0x34);
    STATUS_SET(0, 0, 1, 0, 0);
    RUN("1100 001 10000 0 000"); // ABCD D0, D1
    DATA_CHECK(0, 0x12);
    DATA_CHECK(1, 0x46);
    BCD_STATUS_CHECK(0, 0, 0);
    PASS();
}

TEST test_abcd_carry_lo()
{
    DATA(0, 0x19);
    DATA(1, 0x28);
    STATUS_SET(0, 0, 1, 0, 0);
    RUN("1100 001 10000 0 000");
    DATA_CHECK(1, 0x47);
    BCD_STATUS_CHECK(0, 0, 0);
    PASS();
}

TEST test_abcd_carry_hi()
{
    DATA(0, 0x75);
    DATA(1, 0x50);
    STATUS_SET(0, 0, 1, 0, 0);
    RUN("1100 001 10000 0 000");
    DATA_CHECK(1, 0x25);
    BCD_STATUS_CHECK(1, 0, 1);
    PASS();
}

TEST test_abcd_extend()
{
    DATA(0, 0x15);
    DATA(1, 0x26);
    STATUS_SET(1, 0, 1, 0, 0);
    RUN("1100 001 10000 0 000");
    DATA_CHECK(1, 0x42);
    BCD_STATUS_CHECK(0, 0, 0);
    PASS();
}

TEST test_abcd_zero_keeps_z_set()
{
    DATA(0, 0x99);
    DATA(1, 0x00);
    STATUS_SET(1, 0, 1, 0, 0);
    RUN("1100 001 10000 0 000");
    DATA_CHECK(1, 0x00);
    BCD_STATUS_CHECK(1, 1, 1);
    PASS();
}

TEST test_abcd_zero_keeps_z_clear()
{
    DATA(0, 0x50);
    DATA(1, 0x50);
    STATUS_SET(0, 0, 0, 0, 0);
    RUN("1100 001 10000 0 000");
    DATA_CHECK(1, 0x00);
    BCD_STATUS_CHECK(1, 0, 1);
    PASS();
}

TEST test_abcd_upper_bytes_untouched()
{
    DATA(0, 0xFFFFFF01);
    DATA(1, 0x12345608);
    STATUS_SET(0, 0, 0, 0, 0);
    RUN("1100 001 10000 0 000");
    DATA_CHECK(0, 0xFFFFFF01);
    DATA_CHECK(1, 0x12345609);
    PASS();
}

TEST test_abcd_predec()
{
    ADDR(0, 0x1001);
    ADDR(1, 0x2001);
    MEM(0x1000, 0x45);
    MEM(0x2000, 0x38);
    STATUS_SET(0, 0, 1, 0, 0);
    RUN("1100 001 10000 1 000"); // ABCD -(A0), -(A1)
    MEM_CHECK(0x1000, 0x45);
    MEM_CHECK(0x2000, 0x83);
    ADDR_CHECK(0, 0x1000);
    ADDR_CHECK(1, 0x2000);
    BCD_STATUS_CHECK(0, 0, 0);
    PASS();
}

TEST test_sbcd_basic()
{
    DATA(0, 0x12);
    DATA(1, 0x46);
    STATUS_SET(0, 0, 1, 0, 0);
    RUN("1000 001 10000 0 000"); // SBCD D0, D1
    DATA_CHECK(0, 0x12);
    DATA_CHECK(1, 0x34);
    BCD_STATUS_CHECK(0, 0, 0);
    PASS();
}

TEST test_sbcd_borrow_lo()
{
    DATA(0, 0x15);
    DATA(1, 0x42);
    STATUS_SET(0, 0, 1, 0, 0);
    RUN("1000 001 10000 0 000");
    DATA_CHECK(1, 0x27);
    BCD_STATUS_CHECK(0, 0, 0);
    PASS();
}

TEST test_sbcd_borrow_hi()
{
    DATA(0, 0x20);
    DATA(1, 0x10);
    STATUS_SET(0, 0, 1, 0, 0);
    RUN("1000 001 10000 0 000");
    DATA_CHECK(1, 0x90);
    BCD_STATUS_CHECK(1, 0, 1);
    PASS();
}

TEST test_sbcd_extend()
{
    DATA(0, 0x25);
    DATA(1, 0x50);
    STATUS_SET(1, 0, 1, 0, 0);
    RUN("1000 001 10000 0 000");
    DATA_CHECK(1, 0x24);
    BCD_STATUS_CHECK(0, 0, 0);
    PASS();
}

TEST test_sbcd_zero_keeps_z_set()
{
    DATA(0, 0x25);
    DATA(1, 0x25);
    STATUS_SET(0, 0, 1, 0, 0);
    RUN("1000 001 10000 0 000");
    DATA_CHECK(1, 0x00);
    BCD_STATUS_CHECK(0, 1, 0);
    PASS();
}

TEST test_sbcd_zero_keeps_z_clear()
{
    DATA(0, 0x24);
    DATA(1, 0x25);
    STATUS_SET(1, 0, 0, 0, 0);
    RUN("1000 001 10000 0 000");
    DATA_CHECK(1, 0x00);
    BCD_STATUS_CHECK(0, 0, 0);
    PASS();
}

TEST test_sbcd_upper_bytes_untouched()
{
    DATA(0, 0xFFFFFF01);
    DATA(1, 0x12345609);
    STATUS_SET(0, 0, 0, 0, 0);
    RUN("1000 001 10000 0 000");
    DATA_CHECK(0, 0xFFFFFF01);
    DATA_CHECK(1, 0x12345608);
    PASS();
}

TEST test_sbcd_predec()
{
    ADDR(0, 0x1001);
    ADDR(1, 0x2001);
    MEM(0x1000, 0x38);
    MEM(0x2000, 0x83);
    STATUS_SET(0, 0, 1, 0, 0);
    RUN("1000 001 10000 1 000"); // SBCD -(A0), -(A1)
    MEM_CHECK(0x2000, 0x45);
    MEM_CHECK(0x1000, 0x38);
    ADDR_CHECK(0, 0x1000);
    ADDR_CHECK(1, 0x2000);
    BCD_STATUS_CHECK(0, 0, 0);
    PASS();
}

TEST test_nbcd_basic()
{
    DATA(0, 0x25);
    STATUS_SET(0, 0, 1, 0, 0);
    RUN("0100100000 000 000"); // NBCD D0
    DATA_CHECK(0, 0x75);
    BCD_STATUS_CHECK(1, 0, 1);
    PASS();
}

TEST test_nbcd_extend()
{
    DATA(0, 0x01);
    STATUS_SET(1, 0, 1, 0, 0);
    RUN("0100100000 000 000");
    DATA_CHECK(0, 0x98);
    BCD_STATUS_CHECK(1, 0, 1);
    PASS();
}

TEST test_nbcd_zero_extend()
{
    DATA(0, 0x00);
    STATUS_SET(1, 0, 1, 0, 0);
    RUN("0100100000 000 000");
    DATA_CHECK(0, 0x99);
    BCD_STATUS_CHECK(1, 0, 1);
    PASS();
}

TEST test_nbcd_zero_keeps_z_set()
{
    DATA(0, 0x00);
    STATUS_SET(0, 0, 1, 0, 0);
    RUN("0100100000 000 000");
    DATA_CHECK(0, 0x00);
    BCD_STATUS_CHECK(0, 1, 0);
    PASS();
}

TEST test_nbcd_zero_keeps_z_clear()
{
    DATA(0, 0x00);
    STATUS_SET(0, 0, 0, 0, 0);
    RUN("0100100000 000 000");
    DATA_CHECK(0, 0x00);
    BCD_STATUS_CHECK(0, 0, 0);
    PASS();
}

TEST test_nbcd_upper_bytes_untouched()
{
    DATA(0, 0x12345650);
    STATUS_SET(0, 0, 0, 0, 0);
    RUN("0100100000 000 000");
    DATA_CHECK(0, 0x12345650);
    BCD_STATUS_CHECK(1, 0, 1);
    PASS();
}

TEST test_nbcd_ind()
{
    ADDR(0, 0x1000);
    MEM(0x1000, 0x37);
    STATUS_SET(0, 0, 1, 0, 0);
    RUN("0100100000 010 000"); // NBCD (A0)
    MEM_CHECK(0x1000, 0x63);
    ADDR_CHECK(0, 0x1000);
    BCD_STATUS_CHECK(1, 0, 1);
    PASS();
}

SUITE(test_suite_instructions_bcd)
{
    SET_SETUP(setup, NULL);
    SET_TEARDOWN(teardown, NULL);

    RUN_TEST(test_abcd_basic);
    RUN_TEST(test_abcd_carry_lo);
    RUN_TEST(test_abcd_carry_hi);
    RUN_TEST(test_abcd_extend);
    RUN_TEST(test_abcd_zero_keeps_z_set);
    RUN_TEST(test_abcd_zero_keeps_z_clear);
    RUN_TEST(test_abcd_upper_bytes_untouched);
    RUN_TEST(test_abcd_predec);

    RUN_TEST(test_sbcd_basic);
    RUN_TEST(test_sbcd_borrow_lo);
    RUN_TEST(test_sbcd_borrow_hi);
    RUN_TEST(test_sbcd_extend);
    RUN_TEST(test_sbcd_zero_keeps_z_set);
    RUN_TEST(test_sbcd_zero_keeps_z_clear);
    RUN_TEST(test_sbcd_upper_bytes_untouched);
    RUN_TEST(test_sbcd_predec);

    RUN_TEST(test_nbcd_basic);
    RUN_TEST(test_nbcd_extend);
    RUN_TEST(test_nbcd_zero_extend);
    RUN_TEST(test_nbcd_zero_keeps_z_set);
    RUN_TEST(test_nbcd_zero_keeps_z_clear);
    RUN_TEST(test_nbcd_upper_bytes_untouched);
    RUN_TEST(test_nbcd_ind);
}
