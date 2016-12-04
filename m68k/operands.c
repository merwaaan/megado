#include <stdio.h>
#include <stdlib.h>

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


void operand_free(Operand* operand)
{
    free(operand);
}
