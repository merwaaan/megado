#include <greatest.h>
#include <m68k/m68k.h>
#include <stdbool.h>

#include "globals.h"

TEST test_and_b()
{
    DATA(2, 0xFE800174);
    DATA(3, 0x25CFB7DD);
    RUN("1100 011 0 00 000010"); // AND.b D2, D3
    DATA_CHECK(3, 0x25CFB754);
    PASS();
}

TEST test_and_w()
{
    DATA(2, 0xFE800174);
    DATA(3, 0x25CFB7DD);
    RUN("1100 011 0 01 000010"); // AND.w D2, D3
    DATA_CHECK(3, 0x25CF0154);
    PASS();
}

TEST test_and_l()
{
    DATA(2, 0xFE800174);
    DATA(3, 0x25CFB7DD);
    RUN("1100 011 0 10 000010"); // AND.l D2, D3
    DATA_CHECK(3, 0x24800154);
    PASS();
}

TEST test_eor_b()
{
    DATA(4, 0x25CFB7DD);
    DATA(1, 0xFE800174);
    RUN("1011 100 1 00 000001"); // EOR.b D4, D1
    DATA_CHECK(1, 0xFE8001A9);
    PASS();
}

TEST test_eor_w()
{
    DATA(4, 0x25CFB7DD);
    DATA(1, 0xFE800174);
    RUN("1011 100 1 01 000001"); // EOR.w D4, D1
    DATA_CHECK(1, 0xFE80B6A9);
    PASS();
}

TEST test_eor_l()
{
    DATA(4, 0x25CFB7DD);
    DATA(1, 0xFE800174);
    RUN("1011 100 1 10 000001"); // EOR.l D4, D1
    DATA_CHECK(1, 0xDB4FB6A9);
    PASS();
}

TEST test_or_b()
{
    DATA(4, 0x25CFB7DD);
    DATA(1, 0xFE800174);
    RUN("1000 001 0 00 000100"); // OR.b D4, D1
    DATA_CHECK(1, 0xFE8001FD);
    PASS();
}

TEST test_or_w()
{
    DATA(4, 0x25CFB7DD);
    DATA(1, 0xFE800174);
    RUN("1000 001 0 01 000100"); // OR.w D4, D1
    DATA_CHECK(1, 0xFE80B7FD);
    PASS();
}

TEST test_or_l()
{
    DATA(4, 0x25CFB7DD);
    DATA(1, 0xFE800174);
    RUN("1000 001 0 10 000100"); // OR.l D4, D1
    DATA_CHECK(1, 0xFFCFB7FD);
    PASS();
}

TEST test_andi_b()
{
    DATA(0, 0x12345678);
    STATUS_SET(1, 1, 1, 1, 1);
    MEM_W(2, 0x000F);
    RUN("00000010 00 000000"); // ANDI.b #$0F, D0
    DATA_CHECK(0, 0x12345608);
    STATUS_CHECK(1, 0, 0, 0, 0);
    PASS();
}

TEST test_andi_w()
{
    DATA(0, 0x1234F678);
    STATUS_SET(1, 0, 1, 1, 1);
    MEM_W(2, 0x8F00);
    RUN("00000010 01 000000"); // ANDI.w #$8F00, D0
    DATA_CHECK(0, 0x12348600);
    STATUS_CHECK(1, 1, 0, 0, 0);
    PASS();
}

TEST test_andi_l()
{
    DATA(0, 0x12345678);
    STATUS_SET(1, 1, 0, 1, 1);
    MEM_L(2, 0xEDCBA987);
    RUN("00000010 10 000000"); // ANDI.l #$EDCBA987, D0
    DATA_CHECK(0, 0);
    STATUS_CHECK(1, 0, 1, 0, 0);
    PASS();
}

TEST test_andi_indirect()
{
    ADDR(1, 0x1000);
    MEM_W(0x1000, 0x1234);
    STATUS_SET(1, 1, 1, 1, 1);
    MEM_W(2, 0xFF00);
    RUN("00000010 01 010001"); // ANDI.w #$FF00, (A1)
    MEM_CHECK_W(0x1000, 0x1200);
    ADDR_CHECK(1, 0x1000);
    STATUS_CHECK(1, 0, 0, 0, 0);
    PASS();
}

TEST test_andi_absolute_short()
{
    MEM(0x2000, 0x5A);
    STATUS_SET(1, 1, 1, 1, 1);
    MEM_W(2, 0x00F0);
    MEM_W(4, 0x2000);
    RUN("00000010 00 111000"); // ANDI.b #$F0, ($2000).w
    MEM_CHECK(0x2000, 0x50);
    CHECK(6, m->pc);
    STATUS_CHECK(1, 0, 0, 0, 0);
    PASS();
}

TEST test_andi_ccr()
{
    m->status = 0x271F;
    MEM_W(2, 0x0015);
    RUN("0000001000111100"); // ANDI #$15, CCR
    CHECK(0x2715, m->status);
    PASS();
}

TEST test_andi_sr()
{
    m->status = 0x2715;
    MEM_W(2, 0xF8FF);
    RUN("0000001001111100"); // ANDI #$F8FF, SR
    CHECK(0x2015, m->status);
    PASS();
}

TEST test_ori_b()
{
    DATA(1, 0x12345600);
    STATUS_SET(1, 0, 1, 1, 1);
    MEM_W(2, 0x0081);
    RUN("00000000 00 000001"); // ORI.b #$81, D1
    DATA_CHECK(1, 0x12345681);
    STATUS_CHECK(1, 1, 0, 0, 0);
    PASS();
}

TEST test_ori_w()
{
    DATA(1, 0xFFFF0000);
    STATUS_SET(1, 1, 0, 1, 1);
    MEM_W(2, 0x0000);
    RUN("00000000 01 000001"); // ORI.w #$0000, D1
    DATA_CHECK(1, 0xFFFF0000);
    STATUS_CHECK(1, 0, 1, 0, 0);
    PASS();
}

TEST test_ori_l()
{
    DATA(1, 0x01020304);
    STATUS_SET(1, 1, 1, 1, 1);
    MEM_L(2, 0x10203040);
    RUN("00000000 10 000001"); // ORI.l #$10203040, D1
    DATA_CHECK(1, 0x11223344);
    STATUS_CHECK(1, 0, 0, 0, 0);
    PASS();
}

TEST test_ori_predec()
{
    ADDR(2, 0x1004);
    MEM_L(0x1000, 0x12340000);
    STATUS_SET(1, 1, 1, 1, 1);
    MEM_L(2, 0x0000FFFF);
    RUN("00000000 10 100010"); // ORI.l #$0000FFFF, -(A2)
    MEM_CHECK_L(0x1000, 0x1234FFFF);
    ADDR_CHECK(2, 0x1000);
    STATUS_CHECK(1, 0, 0, 0, 0);
    PASS();
}

TEST test_ori_ccr()
{
    m->status = 0x2700;
    MEM_W(2, 0x000A);
    RUN("0000000000111100"); // ORI #$0A, CCR
    CHECK(0x270A, m->status);
    PASS();
}

TEST test_ori_sr()
{
    m->status = 0x2015;
    MEM_W(2, 0x0700);
    RUN("0000000001111100"); // ORI #$0700, SR
    CHECK(0x2715, m->status);
    PASS();
}

TEST test_ori_sr_unused_bits()
{
    m->status = 0x2700;
    MEM_W(2, 0xFFFF);
    RUN("0000000001111100"); // ORI #$FFFF, SR
    CHECK(0xA71F, m->status);
    PASS();
}

TEST test_eori_b()
{
    DATA(2, 0x123456FF);
    STATUS_SET(1, 1, 0, 1, 1);
    MEM_W(2, 0x00FF);
    RUN("00001010 00 000010"); // EORI.b #$FF, D2
    DATA_CHECK(2, 0x12345600);
    STATUS_CHECK(1, 0, 1, 0, 0);
    PASS();
}

TEST test_eori_w()
{
    DATA(2, 0x12345678);
    STATUS_SET(1, 0, 1, 1, 1);
    MEM_W(2, 0xFFFF);
    RUN("00001010 01 000010"); // EORI.w #$FFFF, D2
    DATA_CHECK(2, 0x1234A987);
    STATUS_CHECK(1, 1, 0, 0, 0);
    PASS();
}

TEST test_eori_l()
{
    DATA(2, 0x12345678);
    STATUS_SET(1, 0, 1, 1, 1);
    MEM_L(2, 0xF0F0F0F0);
    RUN("00001010 10 000010"); // EORI.l #$F0F0F0F0, D2
    DATA_CHECK(2, 0xE2C4A688);
    STATUS_CHECK(1, 1, 0, 0, 0);
    PASS();
}

TEST test_eori_postinc()
{
    ADDR(3, 0x1000);
    MEM(0x1000, 0x0F);
    STATUS_SET(1, 0, 1, 1, 1);
    MEM_W(2, 0x00FF);
    RUN("00001010 00 011011"); // EORI.b #$FF, (A3)+
    MEM_CHECK(0x1000, 0xF0);
    ADDR_CHECK(3, 0x1001);
    STATUS_CHECK(1, 1, 0, 0, 0);
    PASS();
}

TEST test_eori_ccr()
{
    m->status = 0x2715;
    MEM_W(2, 0x001F);
    RUN("0000101000111100"); // EORI #$1F, CCR
    CHECK(0x270A, m->status);
    PASS();
}

TEST test_eori_sr()
{
    m->status = 0x2700;
    MEM_W(2, 0x0011);
    RUN("0000101001111100"); // EORI #$0011, SR
    CHECK(0x2711, m->status);
    PASS();
}

TEST test_not_b()
{
    DATA(7, 0xFE800174);
    RUN("01000110 00 000111"); // NOT.b D7
    DATA_CHECK(7, 0xFE80018B);
    PASS();
}

TEST test_not_w()
{
    DATA(7, 0xFE800174);
    RUN("01000110 01 000111"); // NOT.w D7
    DATA_CHECK(7, 0xFE80FE8B);
    PASS();
}

TEST test_not_l()
{
    DATA(7, 0xFE800174);
    RUN("01000110 10 000111"); // NOT.l D7
    DATA_CHECK(7, 0x017FFE8B);
    PASS();
}

TEST test_tst_b_true()
{
    DATA(4, 0xFE800100);
    RUN("01001010 00 000100"); // TST.b D4
    CHECK(true, ZERO(m));
    PASS();
}

TEST test_tst_b_false()
{
    DATA(4, 0xFE80010C);
    RUN("01001010 00 000100"); // TST.b D4
    CHECK(false, ZERO(m));
    PASS();
}

TEST test_tst_w_true()
{
    DATA(4, 0xFAAF0000);
    RUN("01001010 01 000100"); // TST.w D4
    CHECK(true, ZERO(m));
    PASS();
}

TEST test_tst_w_false()
{
    DATA(4, 0xFAAF0FF0);
    RUN("01001010 01 000100"); // TST.w D4
    CHECK(false, ZERO(m));
    PASS();
}

TEST test_tst_l_true()
{
    DATA(4, 0);
    RUN("01001010 10 000100"); // TST.l D4
    CHECK(true, ZERO(m));
    PASS();
}

TEST test_tst_l_false()
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

    RUN_TEST(test_andi_b);
    RUN_TEST(test_andi_w);
    RUN_TEST(test_andi_l);
    RUN_TEST(test_andi_indirect);
    RUN_TEST(test_andi_absolute_short);
    RUN_TEST(test_andi_ccr);
    RUN_TEST(test_andi_sr);

    RUN_TEST(test_ori_b);
    RUN_TEST(test_ori_w);
    RUN_TEST(test_ori_l);
    RUN_TEST(test_ori_predec);
    RUN_TEST(test_ori_ccr);
    RUN_TEST(test_ori_sr);
    RUN_TEST(test_ori_sr_unused_bits);

    RUN_TEST(test_eori_b);
    RUN_TEST(test_eori_w);
    RUN_TEST(test_eori_l);
    RUN_TEST(test_eori_postinc);
    RUN_TEST(test_eori_ccr);
    RUN_TEST(test_eori_sr);

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
