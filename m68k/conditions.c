#include <stdlib.h>

#include "instruction.h"
#include "m68k.h"
#include "operands.h"

uint32_t True(Operand* this)
{
    return 1;
}

uint32_t False(Operand* this)
{
    return 0;
}

uint32_t High(Operand* this)
{
    return ZERO(this->instruction->context) + CARRY(this->instruction->context) == 0; // TODO
}

uint32_t LowOrSame(Operand* this)
{
    return ZERO(this->instruction->context) | CARRY(this->instruction->context);
}

uint32_t CarryClear(Operand* this)
{
    return CARRY(this->instruction->context) == 0;
}

uint32_t CarrySet(Operand* this)
{
    return CARRY(this->instruction->context) == 1;
}

uint32_t NotEqual(Operand* this)
{
    return ZERO(this->instruction->context) == 0;
}

uint32_t Equal(Operand* this)
{
    return ZERO(this->instruction->context) == 1;
}

uint32_t OverflowClear(Operand* this)
{
    return OVERFLOW(this->instruction->context) == 0;
}

uint32_t OverflowSet(Operand* this)
{
    return OVERFLOW(this->instruction->context) == 1;
}

uint32_t Plus(Operand* this)
{
    return NEGATIVE(this->instruction->context) == 0;
}

uint32_t Minus(Operand* this)
{
    return NEGATIVE(this->instruction->context) == 1;
}

uint32_t GreaterOrEqual(Operand* this)
{
    return 0; // TODO
}

uint32_t LessThan(Operand* this)
{
    return 0; // TODO
}

uint32_t GreaterThan(Operand* this)
{
    return 0; // TODO
}

uint32_t LessOrEqual(Operand* this)
{
    return 0; // TODO
}

GetFunc conditions[] = {
    True,
    False,
    High,
    LowOrSame,
    CarryClear,
    CarrySet,
    NotEqual,
    Equal,
    OverflowClear,
    OverflowSet,
    Plus,
    Minus,
    GreaterOrEqual,
    LessThan,
    GreaterThan,
    LessOrEqual
};

Operand* operand_make_condition(int pattern, Instruction* instr)
{
    Operand* op = calloc(1, sizeof(Operand));
    op->type = Condition;
    op->get = conditions[pattern & 0xF];
    op->instruction = instr;
    return op;
}
