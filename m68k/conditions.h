#pragma once

#include <stdbool.h>

typedef bool(*ConditionFunc)();

bool False()
{
    return false;
}

bool True()
{
    return true;
}
