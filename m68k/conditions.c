#include <stdlib.h>

#include "conditions.h"
#include "instruction.h"
#include "m68k.h"
#include "operands.h"

bool False(M68k* context)
{
    return false;
}

bool True(M68k* context)
{
    return true;
}

bool High(M68k* context)
{
    return ZERO(context) + CARRY(context) == 0; // TODO
}

bool LowOrSame(M68k* context)
{
    return ZERO(context) | CARRY(context);
}

bool CarryClear(M68k* context)
{
    return !CARRY(context);
}

bool CarrySet(M68k* context)
{
    return CARRY(context);
}

bool NotEqual(M68k* context)
{
    return !ZERO(context);
}

bool Equal(M68k* context)
{
    return ZERO(context);
}

bool OverflowClear(M68k* context)
{
    return !OVERFLOW(context);
}

bool OverflowSet(M68k* context)
{
    return OVERFLOW(context);
}

bool Plus(M68k* context)
{
    return !NEGATIVE(context);
}

bool Minus(M68k* context)
{
    return NEGATIVE(context);
}

bool GreaterOrEqual(M68k* context)
{
    return NEGATIVE(context) & OVERFLOW(context) | !NEGATIVE(context) & !OVERFLOW(context);
}

bool LessThan(M68k* context)
{
    return NEGATIVE(context) & !OVERFLOW(context) | !NEGATIVE(context) & OVERFLOW(context);
}

bool GreaterThan(M68k* context)
{
    return NEGATIVE(context) & OVERFLOW(context) & !ZERO(context) | !NEGATIVE(context) & !OVERFLOW(context) & !ZERO(context);
}

bool LessOrEqual(M68k* context)
{
    return ZERO(context) | NEGATIVE(context) & !OVERFLOW(context) | !NEGATIVE(context) & OVERFLOW(context);
}

static Condition all_conditions[] = {
    { True, "T" },
    { False, "F" },
    { High, "HI" },
    { LowOrSame, "LS" },
    { CarryClear, "CC" },
    { CarrySet, "CS" },
    { NotEqual, "NE" },
    { Equal, "EQ" },
    { OverflowClear, "VC" },
    { OverflowSet, "VS" },
    { Plus, "PL" },
    { Minus, "MI" },
    { GreaterOrEqual, "GE" },
    { LessThan, "LT" },
    { GreaterThan, "GT" },
    { LessOrEqual, "LE" }
};

Condition* condition_get(int pattern)
{
    return &all_conditions[pattern];
}
