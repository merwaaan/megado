#pragma once

struct Instruction;
typedef struct Instruction Instruction;

typedef struct M68k {
    uint16_t* memory; // TODO externalize memory

	uint32_t data_registers[8];
	uint32_t address_registers[7];
	uint16_t flags;
	uint32_t sp;
	uint32_t pc;

	Instruction** opcode_table;
} M68k;

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
