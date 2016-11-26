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

void m68k_init();

char* instruction_tostring(Instruction);
