#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "bit_utils.h"
#include "operands.h"

struct Condition;
struct Instruction;
struct M68k;

// Instruction implementations are passed the instruction's data and the CPU context
typedef uint8_t(InstructionFunc)(struct Instruction*, struct M68k*);

typedef struct Instruction
{
    char name[16];

    // Implementation
    InstructionFunc* func;

    // Operands
    Operand src;
    Operand dst;

    // Size of the operation (byte, word, long)
    Size size;

    // Base execution time (fixed part of the instruction's timing)
    //
    // The implementation will return an additional context-dependent
    // execution time that must be added to obtain the total execution
    // time.
    //
    // e.g. for ROL.b, base time is 6
    //                 total time is 6+2n where n is the number of bits to rotate
    //      -> the implementation will return 2n
    uint8_t base_cycles;

    // Some instructions require extra data
    union
    {
        struct Condition* condition; // Branching condition index (see conditions.h)
    };
} Instruction;

void instruction_init(Instruction*, char* name, InstructionFunc func);

// Generates the appropriate instruction from an opcode.
void instruction_generate(Instruction*, uint16_t opcode);

// Executes the given instruction and returns the elapsed cycles.
uint8_t instruction_execute(Instruction*, struct M68k*);

// Placeholder instruction implementation
uint8_t not_implemented(Instruction* i, struct M68k*);
