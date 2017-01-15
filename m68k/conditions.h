#pragma once

#include <stdbool.h>

typedef bool(*ConditionFunc)(struct M68k*);

typedef struct Condition
{
    // Implementation
    ConditionFunc func;

    char* mnemonics;
} Condition;

Condition* condition_get(int pattern);
