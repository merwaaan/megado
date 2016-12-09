#pragma once

#include <stdbool.h>
#include <stdint.h>

struct M68k;
struct Operand;
struct Instruction;

typedef void (InstructionFunc)(struct Instruction*);

typedef struct Instruction {
	char* name;

	InstructionFunc* func;

    long size;

	struct Operand** operands;
	uint8_t operand_count;

	struct M68k* context;
} Instruction;


bool instruction_valid(Instruction* instr);

Instruction* instruction_make(struct M68k* context, char* name, InstructionFunc func);
void instruction_free(Instruction* instr);