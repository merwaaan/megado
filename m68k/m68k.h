#pragma once

#include <stdint.h>

#include "bit_utils.h"

#define CARRY_BIT 0
#define OVERFLOW_BIT 1
#define ZERO_BIT 2
#define NEGATIVE_BIT 3
#define EXTENDED_BIT 4

#define CARRY(context) BIT(context->flags, CARRY_BIT)
#define OVERFLOW(context) BIT(context->flags, OVERFLOW_BIT)
#define ZERO(context) BIT(context->flags, ZERO_BIT)
#define NEGATIVE(context) BIT(context->flags, NEGATIVE_BIT)
#define EXTENDED(context) BIT(context->flags, EXTENDED_BIT)

#define CARRY_SET(context, b) context->flags = BIT_CHG(context->flags, CARRY_BIT, b)
#define OVERFLOW_SET(context, b) context->flags = BIT_CHG(context->flags, OVERFLOW_BIT, b)
#define ZERO_SET(context, b) context->flags = BIT_CHG(context->flags, ZERO_BIT, b)
#define NEGATIVE_SET(context, b) context->flags = BIT_CHG(context->flags, NEGATIVE_BIT, b)
#define EXTENDED_SET(context, b) context->flags = BIT_CHG(context->flags, EXTENDED_BIT, b)

struct Instruction;

typedef struct M68k {
    uint16_t* memory; // TODO externalize memory

	int32_t data_registers[8];
	uint32_t address_registers[7];
	uint16_t flags;
	uint32_t sp;
	uint32_t pc;

	struct Instruction** opcode_table;
} M68k;

typedef struct Instruction* (GenFunc)(uint16_t opcode, M68k* context);

typedef struct {
	uint16_t pattern;
	uint16_t mask;
	GenFunc* generator;
} Pattern;

M68k* m68k_init();
void m68k_free(M68k*);
void m68k_execute(M68k*, uint16_t opcode);

char* instruction_tostring(struct Instruction*);
