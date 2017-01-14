#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "bit_utils.h"

struct Condition;
struct Instruction;
struct M68k;
struct Operand;

typedef void (InstructionFunc)(struct Instruction*);

typedef struct Instruction {
    char* name;

    // The M68000 instance that the instruction is bound to
    struct M68k* context;

    // Implementation
    InstructionFunc* func;

    // Operands
    struct Operand* src;
    struct Operand* dst;

    // Data size (byte, word, long)
    Size size;

    // Instruction length in bytes (depends on the operands' length)
    uint8_t length;

    // Some instruction require extra data
    union
    {
        struct Condition* condition; // Branching condition index (see conditions.h)
    };
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
