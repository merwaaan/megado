#pragma once

struct M68k;
struct Operand;
struct Instruction;

typedef void (InstructionFunc)(struct Instruction*);

typedef struct Instruction {
	char* name;

	InstructionFunc* func;

    uint8_t size;

	struct Operand* operands;
	uint8_t operand_count;

	struct M68k* context;
} Instruction;
