#include <stdio.h>
#include <stdlib.h>

#include "instruction.h"
#include "operands.h"
#include "m68k.h"

// The 'instr_address' argument is the address of the instruction that the operand is part of.
// We need to pass it since some addressing modes are context-dependent.
int operand_tostring(Operand* operand, uint32_t instr_address, char* buffer)
{
    if (operand == NULL)
        return 0;

    switch (operand->type)
    {
    case DataRegister:
        return sprintf(buffer, "D%d", operand->n);
    case AddressRegister:
        return sprintf(buffer, "A%d", operand->n);
    case AddressRegisterIndirect:
        return sprintf(buffer, "(A%d)", operand->n);
    case AddressRegisterIndirectPreDec:
        return sprintf(buffer, "-(A%d)", operand->n);
    case AddressRegisterIndirectPostInc:
        return sprintf(buffer, "(A%d)+", operand->n);
    case AddressRegisterIndirectDisplacement:
        return sprintf(buffer, "TODO (%010x,A%d)", GET_RELATIVE(operand, instr_address), operand->n);
    case AddressRegisterIndirectIndexed:
        return sprintf(buffer, "TODO %d(A%d, D%d)", operand->n, operand->n, operand->n);
    case ProgramCounterDisplacement:
        return sprintf(buffer, "TODO (%010x,PC)", GET_RELATIVE(operand, instr_address));
    case ProgramCounterIndexed:
        return sprintf(buffer, "TODO %d(PC, D%d)", operand->n, operand->n);
    case Immediate:
        return sprintf(buffer, "#$%04x", GET_RELATIVE(operand, instr_address));
    case Value:
        return sprintf(buffer, "#$%04x", operand->n);
    case AbsoluteShort:
        return sprintf(buffer, "($%06x).w", m68k_read_w(operand->instruction->context, instr_address + operand->instruction->base_length));
    case AbsoluteLong:
        return sprintf(buffer, "($%010x).l", m68k_read_l(operand->instruction->context, instr_address + operand->instruction->base_length));
    case BranchingOffset:
        return sprintf(buffer, "$%010x", instr_address + GET_RELATIVE(operand, instr_address) + 2);
    default:
        return 0;
    }
}

Size operand_size(uint8_t pattern)
{
    switch (pattern)
    {
    case 0:
        return Byte;
    case 1:
        return Word;
    case 2:
        return Long;
    default:
        //printf("Invalid operand size %d", pattern); // TODO logging fw?
        return 0;
    }
}

rsize_t operand_size2(uint8_t pattern)
{
    switch (pattern)
    {
    case 1:
        return Byte;
    case 3:
        return Word;
    case 2:
        return Long;
    default:
        //printf("Invalid operand size %d", pattern); // TODO logging fw?
        return 0;
    }
}

Size operand_size3(uint8_t pattern)
{
    switch (pattern)
    {
    case 0:
        return Word;
    case 1:
        return Long;
    default:
        //printf("Invalid operand size %d", pattern); // TODO logging fw?
        return 0;
    }
}

uint8_t operand_sign_extension(uint8_t pattern)
{
    switch (pattern)
    {
    case 2:
        return Word;
    case 3:
        return Long;
    default:
        //printf("Invalid operand size %d", pattern); // TODO logging fw?
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
    case 0x18:
        return operand_make_address_register_indirect_postinc(pattern & 7, instr);
    case 0x20:
        return operand_make_address_register_indirect_predec(pattern & 7, instr);
    case 0x28:
        return operand_make_address_register_indirect_displacement(pattern & 7, instr);
    case 0x38:
        switch (pattern & 7)
        {
        case 0:
            return operand_make_absolute_short(instr);
        case 1:
            return operand_make_absolute_long(instr);
        case 2:
            return operand_make_pc_displacement(instr);
        case 4:
            return operand_make_immediate_value(instr->size, instr);
        }
    default:
        return NULL;
    }
}

void noop(Operand* o, uint32_t instr_address, uint32_t value)
{
}

/*
 * Data register
 */

uint32_t data_get(Operand* o, uint32_t instr_address)
{
    return MASK_ABOVE_INC(o->instruction->context->data_registers[o->n], o->instruction->size);
}

void data_set(Operand* o, uint32_t instr_address, uint32_t value)
{
    o->instruction->context->data_registers[o->n] =
        MASK_BELOW(o->instruction->context->data_registers[o->n], o->instruction->size) |
        MASK_ABOVE_INC(value, o->instruction->size);
}

Operand* operand_make_data_register(int n, Instruction* instr)
{
    Operand* op = calloc(1, sizeof(Operand));
    op->instruction = instr;
    op->type = DataRegister;
    op->get = data_get;
    op->set = data_set;
    op->n = n;
    return op;
}

/*
 * Address register value
 */

uint32_t address_get(Operand* o, uint32_t instr_address)
{
    return MASK_ABOVE_INC(o->instruction->context->address_registers[o->n], o->instruction->size);
}

void address_set(Operand* o, uint32_t instr_address, uint32_t value)
{
    o->instruction->context->address_registers[o->n] =
        MASK_BELOW(o->instruction->context->address_registers[o->n], o->instruction->size) |
        MASK_ABOVE_INC(value, o->instruction->size);
}

Operand* operand_make_address_register(int n, Instruction* instr)
{
    Operand* op = calloc(1, sizeof(Operand));
    op->instruction = instr;
    op->type = AddressRegister;
    op->get = address_get;
    op->set = address_set;
    op->n = n;
    return op;
}

/*
 * Indirect address register
 *
 * The register contains the address of the data in memory.
 */

uint32_t address_indirect_get(Operand* o, uint32_t instr_address)
{
    return m68k_read(o->instruction->context, o->instruction->size, o->instruction->context->address_registers[o->n] & 0xFFFFFF);
}

void address_indirect_set(Operand* o, uint32_t instr_address, uint32_t value)
{
    m68k_write(o->instruction->context, o->instruction->size, o->instruction->context->address_registers[o->n] & 0xFFFFFF, value);
}

Operand* operand_make_address_register_indirect(int n, Instruction* instr)
{
    Operand* op = calloc(1, sizeof(Operand));
    op->instruction = instr;
    op->type = AddressRegisterIndirect;
    op->get = address_indirect_get;
    op->set = address_indirect_set;
    op->n = n;
    return op;
}

/*
 * Post-increment indirect address register
 *
 * The address register is incremented AFTER reading/writing data.
 * The increment value depends on the instruction size.
 */

void address_inc(Operand* o)
{
    o->instruction->context->address_registers[o->n] += size_in_bytes(o->instruction->size);
}

Operand* operand_make_address_register_indirect_postinc(int n, struct Instruction* instr)
{
    Operand* op = calloc(1, sizeof(Operand));
    op->instruction = instr;
    op->type = AddressRegisterIndirectPostInc;
    op->get = address_indirect_get;
    op->set = address_indirect_set;
    op->post = address_inc;
    op->n = n;
    return op;
}

/*
 * Pre-decrement indirect
 *
 * The address register is decremented BEFORE reading/writing data.
 * The decrement value depends on the instruction size.
 */

void address_dec(Operand* o)
{
    o->instruction->context->address_registers[o->n] -= size_in_bytes(o->instruction->size);
}

Operand* operand_make_address_register_indirect_predec(int n, struct Instruction* instr)
{
    Operand* op = calloc(1, sizeof(Operand));
    op->instruction = instr;
    op->type = AddressRegisterIndirectPreDec;
    op->get = address_indirect_get;
    op->set = address_indirect_set;
    op->pre = address_dec;
    op->n = n;
    return op;
}

/*
 * Address indirect with displacement
 *
 * The data is at the stored address + a displacement (extension)
 */

uint32_t address_indirect_displacement_get(Operand* o, uint32_t instr_address)
{
    M68k* m = o->instruction->context;
    uint32_t address = m->address_registers[o->n];
    int16_t displacement = m68k_read_w(m, instr_address + 2);
    return  m68k_read_w(m, address + displacement);
}

void address_indirect_displacement_set(Operand* o, uint32_t instr_address, uint32_t value)
{
    M68k* m = o->instruction->context;
    uint32_t address = m->address_registers[o->n];
    int16_t displacement = m68k_read_w(m, instr_address + 2);
    m68k_write_w(m, address + displacement, value);
}

Operand* operand_make_address_register_indirect_displacement(int n, struct Instruction* instr)
{
    Operand* op = calloc(1, sizeof(Operand));
    op->instruction = instr;
    op->type = AddressRegisterIndirectDisplacement;
    op->get = address_indirect_displacement_get;
    op->set = address_indirect_displacement_set;
    op->n = n;
    return op;
}

/*
* Immediate value encoded in the extension words of an instruction
*/

uint32_t immediate_get_byte(Operand* o, uint32_t instr_address)
{
    return m68k_read_b(o->instruction->context, instr_address + 3);
}

uint32_t immediate_get_word(Operand* o, uint32_t instr_address)
{
    return m68k_read_w(o->instruction->context, instr_address + 2);
}

uint32_t immediate_get_long(Operand* o, uint32_t instr_address)
{
    return m68k_read_l(o->instruction->context, instr_address + 2);
}

Operand* operand_make_immediate_value(Size size, Instruction* instr)
{
    Operand* op = calloc(1, sizeof(Operand));
    op->instruction = instr;
    op->type = Immediate;
    op->set = noop;

    switch (size) {
    case Byte:
        op->get = immediate_get_byte;
        break;
    case Word:
        op->get = immediate_get_word;
        break;
    case Long:
        op->get = immediate_get_long;
        break;
    }

    return op;
}

/*
 * Address encoded in the extension words of an instruction
 */

uint32_t absolute_short_get(Operand* o, uint32_t instr_address)
{
    M68k* m = o->instruction->context;
    uint16_t address = m68k_read_w(m, instr_address + +o->instruction->base_length);
    return  m68k_read_w(m, address); // TODO not sure about w.l
}

Operand* operand_make_absolute_short(Instruction* instr)
{
    Operand* op = calloc(1, sizeof(Operand));
    op->instruction = instr;
    op->type = AbsoluteShort;
    op->get = absolute_short_get;
    op->set = noop;
    return op;
}

uint32_t absolute_long_get(Operand* o, uint32_t instr_address)
{
    M68k* m = o->instruction->context;
    uint32_t address = m68k_read_l(m, instr_address + o->instruction->base_length);
    return  m68k_read_l(m, address); // TODO not sure about w.l
}

Operand* operand_make_absolute_long(Instruction* instr)
{
    Operand* op = calloc(1, sizeof(Operand));
    op->instruction = instr;
    op->type = AbsoluteLong;
    op->get = absolute_long_get;
    op->set = noop;
    return op;
}

/*
 *
 */

uint32_t pc_displacement_word_get(Operand* o, uint32_t instr_address)
{
    M68k* m = o->instruction->context;
    int16_t displacement = m68k_read_w(m, instr_address + 2);
    return m->pc + displacement + 2; // TODO no idea why +2
}

Operand* operand_make_pc_displacement(Instruction* instr)
{
    Operand* op = calloc(1, sizeof(Operand));
    op->type = ProgramCounterDisplacement;
    op->instruction = instr;
    op->get = pc_displacement_word_get;
    op->set = noop;
    return op;
}

/*
* Value directly stored within the opcode
*/

uint32_t value_get(Operand* o, uint32_t instr_address)
{
    return o->n;
}

Operand* operand_make_value(int value, struct Instruction* instr)
{
    Operand* op = calloc(1, sizeof(Operand));
    op->type = Value;
    op->instruction = instr;
    op->n = value;
    op->get = value_get;
    op->set = noop;
    return op;
}

/*
 * Branching offset (specific to control flow isntruction such as Bcc)
 * TODO can't I factor this?
 */

uint32_t branching_offset_byte_get(Operand* o, uint32_t instr_address)
{
    return m68k_read_b(o->instruction->context, instr_address + 1);
}

uint32_t branching_offset_word_get(Operand* o, uint32_t instr_address)
{
    return m68k_read_w(o->instruction->context, instr_address + 2);
}

uint32_t branching_offset_long_get(Operand* o, uint32_t instr_address)
{
    return m68k_read_l(o->instruction->context, instr_address + 2);
}

Operand* operand_make_branching_offset(Instruction* instr, Size size)
{
    Operand* op = calloc(1, sizeof(Operand));
    op->instruction = instr;
    op->type = BranchingOffset;

    op->set = noop;

    switch (size)
    {
    case Byte:
        op->get = branching_offset_byte_get;
        break;
    case Word:
        op->get = branching_offset_byte_get;
        break;
    case Long:
        op->get = branching_offset_byte_get;
        break;
    }

    return op;
}




int operand_length(Operand* operand)
{
    if (operand == NULL)
        return 0;

    switch (operand->type)
    {
    case AbsoluteShort:
    case AddressRegisterIndirectDisplacement:
    case ProgramCounterDisplacement:
        return 2;
    case AbsoluteLong:
        return 4;
    case Immediate:
        if (operand->get == immediate_get_byte || operand->get == immediate_get_word)
            return 2;
        else
            return 4;
    default:
        return 0;
    }
}

// Cycles required to compute an effective address.
// The first value is for Byte/Word operation, the second is for Long operations.
// The order of the entries must match the AddressingMode enum.
int address_calculation_cycles[12][2] = 
{
    { 0, 0 },
    { 0, 0 },
    { 4, 8 },
    { 6, 10 },
    { 4, 8 },
    { 8, 12 },
    { 10, 14 },
    { 8, 12 },
    { 10, 14 },
    { 4, 8 },
    { 8, 12 },
    { 10, 14 }
};

int operand_get_cycles(Operand* o)
{
    return address_calculation_cycles[o->type][o->instruction->size == Long];
}
