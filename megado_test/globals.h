#pragma once

#include <genesis.h>
#include <greatest.h>
#include <m68k/m68k.h>
#include <m68k/map.h>

#define CHECK(a, b) ASSERT_EQ_FMT((unsigned int)(a), (unsigned int)(b), "%#010X")

#define DATA(n, x) m->data_registers[n] = x
#define DATA_CHECK(n, x) CHECK(x, m->data_registers[n])

#define ADDR(n, x) m->address_registers[n] = x
#define ADDR_CHECK(n, x) CHECK(x, m->address_registers[n])

#define MEM(a, x) map_write_b(g->map, a, x);
#define MEM_W(a, x) map_write_w(g->map, a, x);
#define MEM_L(a, x) map_write_l(g->map, a, x);
#define MEM_CHECK(a, x) CHECK(x, map_read_b(g->map, a))
#define MEM_CHECK_W(a, x) CHECK(x, map_read_w(g->map, a))
#define MEM_CHECK_L(a, x) CHECK(x, map_read_l(g->map, a))

#define STATUS_SET(x, n, z, v, c) \
    EXTENDED_SET(m, x);           \
    NEGATIVE_SET(m, n);           \
    ZERO_SET(m, z);               \
    OVERFLOW_SET(m, v);           \
    CARRY_SET(m, c)

#define STATUS_CHECK(x, n, z, v, c) \
    CHECK(x, EXTENDED(m));          \
    CHECK(n, NEGATIVE(m));          \
    CHECK(z, ZERO(m));              \
    CHECK(v, OVERFLOW(m));          \
    CHECK(c, CARRY(m))

#define RUN(opcode)                            \
    m->pc = 0;                                 \
    map_write_w(g->map, 0, bin_parse(opcode)); \
    m68k_step(m)

extern Genesis* g;
extern M68k* m;
extern uint8_t* memory;

void setup(void* data);
void teardown(void* data);

void test_map_init(Map*);
