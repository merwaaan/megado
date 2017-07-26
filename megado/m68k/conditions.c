#include "conditions.h"
#include "m68k.h"

static bool False(struct M68k* context)
{
    return false;
}

static bool True(struct M68k* context)
{
    return true;
}

static bool High(struct M68k* context)
{
    return !ZERO(context) & !CARRY(context);
}

static bool LowOrSame(struct M68k* context)
{
    return ZERO(context) | CARRY(context);
}

static bool CarryClear(struct M68k* context)
{
    return !CARRY(context);
}

static bool CarrySet(struct M68k* context)
{
    return CARRY(context);
}

static bool NotEqual(struct M68k* context)
{
    return !ZERO(context);
}

static bool Equal(struct M68k* context)
{
    return ZERO(context);
}

static bool OverflowClear(struct M68k* context)
{
    return !OVERFLOW(context);
}

static bool OverflowSet(struct M68k* context)
{
    return OVERFLOW(context);
}

static bool Plus(struct M68k* context)
{
    return !NEGATIVE(context);
}

static bool Minus(struct M68k* context)
{
    return NEGATIVE(context);
}

static bool GreaterOrEqual(struct M68k* context)
{
    return (NEGATIVE(context) & OVERFLOW(context)) | (!NEGATIVE(context) & !OVERFLOW(context));
}

static bool LessThan(struct M68k* context)
{
    return (NEGATIVE(context) & !OVERFLOW(context)) | ((!NEGATIVE(context)) & OVERFLOW(context));
}

static bool GreaterThan(struct M68k* context)
{
    return (NEGATIVE(context) & OVERFLOW(context) & !ZERO(context)) | ((!NEGATIVE(context)) & !OVERFLOW(context) & !ZERO(context));
}

static bool LessOrEqual(struct M68k* context)
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
