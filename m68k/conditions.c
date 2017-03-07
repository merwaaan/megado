#include <stdlib.h>

#include "conditions.h"
#include "instruction.h"
#include "m68k.h"
#include "operands.h"

bool False(M68k* context)
{
    return ZERO(context);
}

bool True(M68k* context)
{
    return !ZERO(context);
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
    return CARRY(context) == 0;
}

bool CarrySet(M68k* context)
{
    return CARRY(context) == 1;
}

bool NotEqual(M68k* context)
{
    return ZERO(context) == 0;
}

bool Equal(M68k* context)
{
    return ZERO(context) == 1;
}

bool OverflowClear(M68k* context)
{
    return OVERFLOW(context) == 0;
}

bool OverflowSet(M68k* context)
{
    return OVERFLOW(context) == 1;
}

bool Plus(M68k* context)
{
    return NEGATIVE(context) == 0;
}

bool Minus(M68k* context)
{
    return NEGATIVE(context) == 1;
}

bool GreaterOrEqual(M68k* context)
{
    return 0; // TODO
}

bool LessThan(M68k* context)
{
    return 0; // TODO
}

bool GreaterThan(M68k* context)
{
    return 0; // TODO
}

bool LessOrEqual(M68k* context)
{
    return 0; // TODO
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
