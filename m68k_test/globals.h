#pragma once

#include <m68k/m68k.h>

#define DATA(n, x) m->data_registers[n] = x
#define DATA_CHECK(n, x) mu_assert_int_eq_hex(x, m->data_registers[n])

#define ADDR(n, x) m->address_registers[n] = x
#define ADDR_CHECK(n, x) mu_assert_int_eq_hex(x, m->address_registers[n])

#define MEM(a, x) m68k_write_b(m, a, x);
#define MEM_W(a, x) m68k_write_w(m, a, x);
#define MEM_L(a, x) m68k_write_l(m, a, x);
#define MEM_CHECK(a, x) mu_assert_int_eq_hex(x, m68k_read_b(m, a))
#define MEM_CHECK_W(a, x) mu_assert_int_eq_hex(x, m68k_read_w(m, a))
#define MEM_CHECK_L(a, x) mu_assert_int_eq_hex(x, m68k_read_l(m, a))

#define RUN(opcode) m68k_execute(m, bin_parse(opcode))

// This extends minunit to print asserted values in hexadecimal format
#define mu_assert_int_eq_hex(expected, result) MU__SAFE_BLOCK(\
	int minunit_tmp_e;\
	int minunit_tmp_r;\
	minunit_assert++;\
	minunit_tmp_e = (expected);\
	minunit_tmp_r = (result);\
	if (minunit_tmp_e != minunit_tmp_r) {\
		snprintf(minunit_last_message, MINUNIT_MESSAGE_LEN, "%s failed:\n\t%s:%d: %#08X expected but was %#08X", __func__, __FILE__, __LINE__, minunit_tmp_e, minunit_tmp_r);\
		minunit_status = 1;\
		return;\
	} else {\
		printf(".");\
	}\
)

// This extends minunit to print asserted values in binary format
#define mu_assert_int_eq_bin(expected, result) MU__SAFE_BLOCK(\
	int minunit_tmp_e;\
	int minunit_tmp_r;\
	minunit_assert++;\
	minunit_tmp_e = (expected);\
	minunit_tmp_r = (result);\
	if (minunit_tmp_e != minunit_tmp_r) {\
		snprintf(minunit_last_message, MINUNIT_MESSAGE_LEN, "%s failed:\n\t%s:%d: %#08X expected but was %#08X", __func__, __FILE__, __LINE__, minunit_tmp_e, minunit_tmp_r);\
		minunit_status = 1;\
		return;\
	} else {\
		printf(".");\
	}\
)

// This extends minunit to compare strings
#define mu_assert_str_eq(expected, result) MU__SAFE_BLOCK(\
	char* minunit_tmp_e;\
	char* minunit_tmp_r;\
	minunit_assert++;\
	minunit_tmp_e = (expected);\
	minunit_tmp_r = (result);\
	if (strcmp(minunit_tmp_e, minunit_tmp_r) != 0) {\
		snprintf(minunit_last_message, MINUNIT_MESSAGE_LEN, "%s failed:\n\t%s:%d: \"%s\" expected but was \"%s\"", __func__, __FILE__, __LINE__, minunit_tmp_e, minunit_tmp_r);\
		minunit_status = 1;\
		return;\
	} else {\
		printf(".");\
	}\
)

extern M68k* m;
extern uint8_t* memory;

void setup();
void teardown();
