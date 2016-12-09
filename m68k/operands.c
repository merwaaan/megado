#include <stdio.h>
#include <stdlib.h>

#include "conditions.h"
#include "globals.h"
#include "instruction.h"
#include "operands.h"
#include "m68k.h"

char* operand_tostring(Operand* operand)
{
    static char buffer[1024];

    switch (operand->type)
    {
    case DataRegister:
        sprintf(buffer, "D%d", operand->n);
        break;
    case AddressRegister:
        sprintf(buffer, "A%d", operand->n);
        break;
    case AddressRegisterIndirect:
        sprintf(buffer, "(A%d)", operand->n);
        break;
    default:
        sprintf(buffer, "UNSUPPORTED");
    }

    return buffer;
}

uint8_t operand_size(uint8_t pattern)
{
    switch (pattern)
    {
    case 0:
        return 8;
    case 1:
        return 16;
    case 2:
        return 32;
    default:
        return 0;
    }
}

void operand_free(Operand* operand)
{
    free(operand);
}

Operand* operand_make(uint16_t pattern, Instruction* instr)
{
    switch (pattern & 0x38)
    {
    case 0:
        return operand_make_data_register(pattern & 7, instr);
    case 0x8:
        return operand_make_address_register(pattern & 7, instr);
    case 0x10:
        return operand_make_address_register_indirect(pattern & 7, instr);
    default:
        return NULL;
    }
}

/*
 * Direct data register value
 */

int32_t data_register_get(Operand* this)
{
    return this->instruction->context->data_registers[this->n];
}

void data_register_set(Operand* this, int32_t value)
{
    this->instruction->context->data_registers[this->n] = value;
}

Operand* operand_make_data_register(int n, Instruction* instr)
{
    Operand* op = calloc(1, sizeof(Operand));
    op->type = DataRegister;
    op->get = data_register_get;
    op->set = data_register_set;
    op->n = n;
    op->instruction = instr;
    return op;
}

/*
 * Direct address register value
 */

int32_t address_register_get(Operand* this)
{
    return this->instruction->context->address_registers[this->n];
}

void address_register_set(Operand* this, int32_t value)
{
    this->instruction->context->address_registers[this->n] = value;
}

Operand* operand_make_address_register(int n, Instruction* instr)
{
    Operand* op = calloc(1, sizeof(Operand));
    op->type = AddressRegister;
    op->get = address_register_get;
    op->set = address_register_set;
    op->n = n;
    op->instruction = instr;
    return op;
}

/*
 * Indirect address register value
 *
 * The register contains the address of the data in memory.
 */

int32_t address_register_indirect_get(Operand* this)
{
    return this->instruction->context->memory[this->instruction->context->address_registers[this->n]];
}

void address_register_indirect_set(Operand* this, int32_t value)
{
    this->instruction->context->memory[this->instruction->context->address_registers[this->n]] = value;
}

Operand* operand_make_address_register_indirect(int n, Instruction* instr)
{
    Operand* op = calloc(1, sizeof(Operand));
    op->type = AddressRegisterIndirect;
    op->get = address_register_indirect_get;
    op->set = address_register_indirect_set;
    op->n = n;
    op->instruction = instr;
    return op;
}

/*
* Immediate value encoded within an instruction
*/

int32_t immediate_get(Operand* this)
{
    return this->n;
}

void immediate_set(Operand* this, int32_t value)
{
    this->n = value;
}

Operand* operand_make_immediate(int value, Instruction* instr)
{
    Operand* op = calloc(1, sizeof(Operand));
    op->type = Immediate;
    op->get = immediate_get;
    op->set = immediate_set;
    op->n = value;
    op->instruction = instr;
    return op;
}

/*
* Extension word or long word following an instruction
*/

int32_t extension_get(Operand* this)
{
    uint32_t pc = this->instruction->context->pc;
    uint8_t* m = this->instruction->context->memory;

    if (this->n == 16)
        return (m[pc + 2] << 8) | m[pc + 3];

    return (m[pc + 2] << 24) | (m[pc + 3] << 16) | (m[pc + 4] << 8) | m[pc + 5];
}

void extension_set(Operand* this, int32_t value)
{
}

Operand* operand_make_extension(int size, Instruction* instr)
{
    Operand* op = calloc(1, sizeof(Operand));
    op->type = Extension;
    op->get = extension_get;
    op->set = extension_set;
    op->n = size;
    op->instruction = instr;
    return op;
}

/*
 * Condition
 */

ConditionFunc _conditions[] = {
    True,
    False,
    /*Higher,
    LowerOrSame,
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
    LessOrEqual*/
};

Operand* operand_make_condition(uint8_t pattern)
{
    // TODO handle OoB -> if (pattern > 0XF)
    return _conditions[pattern];
}