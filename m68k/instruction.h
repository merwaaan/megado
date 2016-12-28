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

    struct Operand* src;
    struct Operand* dst;

    struct M68k* context;
} Instruction;

// TODO just make it a char*?
typedef struct DecodedInstruction {
    char* mnemonics;
} DecodedInstruction;

Instruction* instruction_make(struct M68k* context, char* name, InstructionFunc func);
Instruction* instruction_generate(struct M68k* context, uint16_t opcode);
void instruction_free(Instruction* instr);

// Check if an instruction is fully formed, ie. a function have been
// assigned and all operands have been setup
bool instruction_valid(Instruction* instr);
