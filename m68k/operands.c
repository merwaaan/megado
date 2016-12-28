#include <stdio.h>
#include <stdlib.h>

#include "instruction.h"
#include "operands.h"
#include "m68k.h"

int operand_tostring(Operand* operand, char* buffer)
{
    switch (operand->type)
    {
    case Data:
        return sprintf(buffer, "D%d", operand->n);
    case Address:
        return sprintf(buffer, "A%d", operand->n);
    case AddressIndirect:
        return sprintf(buffer, "(A%d)", operand->n);
    case AddressIndirectPreInc:
        return sprintf(buffer, "+(A%d)", operand->n);
    case AddressIndirectPostInc:
        return sprintf(buffer, "(A%d)-", operand->n);
    case AddressIndirectOffset:
        return sprintf(buffer, "%d(A%d)-", operand->n, operand->n); // TODO
    case AddressIndirectIndexOffset:
        return sprintf(buffer, "%d(A%d, D%d)-", operand->n, operand->n, operand->n); // TODO
    case ProgramCounterOffset:
        return sprintf(buffer, "%d(PC)-", operand->n); // TODO
    case ProgramCounterIndexOffset:
        return sprintf(buffer, "%d(PC, D%d)-", operand->n, operand->n); // TODO
    case Immediate:
        return sprintf(buffer, "#%d", operand->n);
    case AbsoluteShort:
        return sprintf(buffer, "(%d).w", operand->n);
    case AbsoluteLong:
        return sprintf(buffer, "(%d).l", operand->n);
    default:
        return sprintf(buffer, "UNSUPPORTED");
    }
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
        return operand_make_data(pattern & 7, instr);
    case 0x8:
        return operand_make_address(pattern & 7, instr);
    case 0x10:
        return operand_make_address_indirect(pattern & 7, instr);
    case 0x18:
        return operand_make_address_indirect_postincrement(pattern & 7, instr);
    case 0x32:
        return operand_make_address_indirect_predecrement(pattern & 7, instr);
    case 0x38:
        switch (pattern & 7)
        {
        case 0:
            return operand_make_absolute_short(instr);
        case 1:
            return operand_make_absolute_long(instr);
        }
    default:
        return NULL;
    }
}

void noop(Operand* this, uint32_t value)
{
}

/*
 * Direct data register value
 */

uint32_t data_get(Operand* this)
{
    return this->instruction->context->data_registers[this->n];
}

void data_set(Operand* this, uint32_t value)
{
    this->instruction->context->data_registers[this->n] = value;
}

Operand* operand_make_data(int n, Instruction* instr)
{
    Operand* op = calloc(1, sizeof(Operand));
    op->type = Data;
    op->get = data_get;
    op->set = data_set;
    op->n = n;
    op->instruction = instr;
    return op;
}

/*
 * Direct address register value
 */

uint32_t address_get(Operand* this)
{
    return this->instruction->context->address_registers[this->n];
}

void address_set(Operand* this, uint32_t value)
{
    this->instruction->context->address_registers[this->n] = value;
}

Operand* operand_make_address(int n, Instruction* instr)
{
    Operand* op = calloc(1, sizeof(Operand));
    op->type = Address;
    op->get = address_get;
    op->set = address_set;
    op->n = n;
    op->instruction = instr;
    return op;
}

/*
 * Indirect address register value
 *
 * The register contains the address of the data in memory.
 */

uint32_t address_indirect_get(Operand* this)
{
    return this->instruction->context->memory[this->instruction->context->address_registers[this->n]];
}

void address_indirect_set(Operand* this, uint32_t value)
{
    this->instruction->context->memory[this->instruction->context->address_registers[this->n]] = value;
}

Operand* operand_make_address_indirect(int n, Instruction* instr)
{
    Operand* op = calloc(1, sizeof(Operand));
    op->type = AddressIndirect;
    op->get = address_indirect_get;
    op->set = address_indirect_set;
    op->n = n;
    op->instruction = instr;
    return op;
}

/*
 * Post-increment indirect
 *
 * The address register is incremented AFTER reading/writing data.
 */

void address_inc(Operand* this)
{
    this->instruction->context->address_registers[this->n] += this->instruction->size;
}

Operand* operand_make_address_indirect_postincrement(int n, struct Instruction* instr)
{
    Operand* op = calloc(1, sizeof(Operand));
    op->type = AddressIndirectPostInc;
    op->get = address_indirect_get;
    op->set = address_indirect_set;
    op->post = address_inc;
    op->n = n;
    op->instruction = instr;
    return op;
}

/*
* Pre-decrement indirect
*
* The address register is decremented BEFORE reading/writing data.
*/

void address_dec(Operand* this)
{
    this->instruction->context->address_registers[this->n] -= this->instruction->size;
}

Operand* operand_make_address_indirect_predecrement(int n, struct Instruction* instr)
{
    Operand* op = calloc(1, sizeof(Operand));
    op->type = AddressIndirectPostInc;
    op->get = address_indirect_get;
    op->set = address_indirect_set;
    op->pre = address_dec;
    op->n = n;
    op->instruction = instr;
    return op;
}

/*
 * Immediate value encoded within an instruction
 */

uint32_t immediate_get(Operand* this)
{
    return this->n;
}

void immediate_set(Operand* this, uint32_t value)
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

uint32_t absolute_short_get(Operand* this)
{
    uint32_t pc = this->instruction->context->pc;
    uint8_t* m = this->instruction->context->memory;

    return (m[pc + 2] << 8) | m[pc + 3];
}

Operand* operand_make_absolute_short(Instruction* instr)
{
    Operand* op = calloc(1, sizeof(Operand));
    op->type = AbsoluteShort;
    op->get = absolute_short_get;
    op->set = noop;
    op->instruction = instr;
    return op;
}

uint32_t absolute_long_get(Operand* this)
{
    uint32_t pc = this->instruction->context->pc;
    uint8_t* m = this->instruction->context->memory;

    return (m[pc + 2] << 24) | (m[pc + 3] << 16) | (m[pc + 4] << 8) | m[pc + 5];
}

Operand* operand_make_absolute_long(Instruction* instr)
{
    Operand* op = calloc(1, sizeof(Operand));
    op->type = AbsoluteLong;
    op->get = absolute_long_get;
    op->set = noop;
    op->instruction = instr;
    return op;
}
