#pragma once

#include <genesis.h>
#include <greatest.h>
#include <m68k/m68k.h>

#define CHECK(a, b) ASSERT_EQ_FMT((unsigned int)(a), (unsigned int)(b), "%#010X")

#define DATA(n, x) m->data_registers[n] = x
#define DATA_CHECK(n, x) CHECK(x, m->data_registers[n])

#define ADDR(n, x) m->address_registers[n] = x
#define ADDR_CHECK(n, x) CHECK(x, m->address_registers[n])

#define MEM(a, x) m68k_write_b(m, a, x);
#define MEM_W(a, x) m68k_write_w(m, a, x);
#define MEM_L(a, x) m68k_write_l(m, a, x);
#define MEM_CHECK(a, x) CHECK(x, m68k_read_b(m, a))
#define MEM_CHECK_W(a, x) CHECK(x, m68k_read_w(m, a))
#define MEM_CHECK_L(a, x) CHECK(x, m68k_read_l(m, a))

#define STATUS_CHECK(x, n, z, v, c) \
    CHECK(x, EXTENDED(m));          \
    CHECK(n, NEGATIVE(m));          \
    CHECK(z, ZERO(m));              \
    CHECK(v, OVERFLOW(m));          \
    CHECK(c, CARRY(m))

#define RUN(opcode)                        \
    m->pc = 0;                             \
    m68k_write_w(m, 0, bin_parse(opcode)); \
    m68k_step(m)

extern Genesis* g;
extern M68k* m;
extern uint8_t* memory;

void setup(void* data);
void teardown(void* data);
