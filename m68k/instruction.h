#pragma once

struct M68k;
struct Operand;

typedef void (InstrFunc)(struct Operand*);

typedef struct Instruction {
	char* name;

	InstrFunc* func;

	struct Operand* operands;
	int operand_count;

	struct M68k* context;
} Instruction;

#define EXECUTE(instruction, cpu) instruction.func(instruction, cpu)
