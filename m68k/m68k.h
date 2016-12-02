#pragma once

#include "operands.h"

struct Instruction;

typedef struct M68k {
    uint16_t* memory; // TODO externalize memory

	uint32_t data_registers[8];
	uint32_t address_registers[7];
	uint16_t flags;
	uint32_t sp;
	uint32_t pc;

	struct Instruction** opcode_table;
} M68k;

typedef void (InstrFunc)(struct Operand*);

typedef struct Instruction {
	char* name;

	InstrFunc* func;

	Operand* operands;
	int operand_count;

	M68k* context;
} Instruction;

#define EXECUTE(instruction, cpu) instruction.func(instruction, cpu)

typedef Instruction* (GenFunc)(uint16_t opcode, M68k* context);

typedef struct {
	uint16_t pattern;
	uint16_t mask;
	GenFunc* generator;
} Pattern;

M68k* m68k_init();
void m68k_free(M68k*);
void m68k_execute(M68k*, uint16_t opcode);

char* instruction_tostring(Instruction);
