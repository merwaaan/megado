#pragma once

#include <stdbool.h>

struct Instruction;

typedef bool(*ConditionFunc)(struct M68k*);

typedef struct Condition
{
    // Implementation
    ConditionFunc func;

    char* mnemonics;
} Condition;

Condition* condition_get(struct Instruction* instr, int pattern);
