#include "conditions.h"
#include "m68k.h"

static bool False(M68k* context)
{
    return false;
}

static bool True(M68k* context)
{
    return true;
}

static bool High(M68k* context)
{
    return !ZERO(context) & !CARRY(context);
}

static bool LowOrSame(M68k* context)
{
    return ZERO(context) | CARRY(context);
}

static bool CarryClear(M68k* context)
{
    return !CARRY(context);
}

static bool CarrySet(M68k* context)
{
    return CARRY(context);
}

static bool NotEqual(M68k* context)
{
    return !ZERO(context);
}

static bool Equal(M68k* context)
{
    return ZERO(context);
}

static bool OverflowClear(M68k* context)
{
    return !OVERFLOW(context);
}

static bool OverflowSet(M68k* context)
{
    return OVERFLOW(context);
}

static bool Plus(M68k* context)
{
    return !NEGATIVE(context);
}

static bool Minus(M68k* context)
{
    return NEGATIVE(context);
}

static bool GreaterOrEqual(M68k* context)
{
    return (NEGATIVE(context) & OVERFLOW(context)) | (!NEGATIVE(context) & !OVERFLOW(context));
}

static bool LessThan(M68k* context)
{
    return (NEGATIVE(context) & !OVERFLOW(context)) | ((!NEGATIVE(context)) & OVERFLOW(context));
}

static bool GreaterThan(M68k* context)
{
    return (NEGATIVE(context) & OVERFLOW(context) & !ZERO(context)) | ((!NEGATIVE(context)) & !OVERFLOW(context) & !ZERO(context));
}

static bool LessOrEqual(M68k* context)
{
    return ZERO(context) | (NEGATIVE(context) & !OVERFLOW(context)) | ((!NEGATIVE(context)) & OVERFLOW(context));
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
