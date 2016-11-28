#pragma once

#include "operands.h"

typedef void (InstrFunc)(Operand *operands);

typedef struct {
	char *name;
	InstrFunc *func;
	Operand *operands;
	int operandCount;
} Instruction;

typedef Instruction (GenFunc)(uint16_t opcode);

typedef struct {
	uint16_t pattern;
	uint16_t mask;
	GenFunc *generator;
} Pattern;

extern Instruction *_instructions;

typedef struct {
	uint32_t data_registers[8];
	uint32_t address_registers[7];
	uint16_t flags;
	uint32_t sp;
	uint32_t pc;
} M68k;

extern M68k _m68k;

void m68k_init();

char* instruction_tostring(Instruction);
