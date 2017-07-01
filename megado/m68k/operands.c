#include <stdio.h>
#include <stdlib.h>

#include "instruction.h"
#include "operands.h"
#include "m68k.h"

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
    case 0x30:
        return operand_make_address_register_indirect_index(pattern & 7, instr);
    case 0x38:
        switch (pattern & 7)
        {
        case 0:
            return operand_make_absolute_short(instr);
        case 1:
            return operand_make_absolute_long(instr);
        case 2:
            return operand_make_pc_displacement(instr);
        case 3:
            return operand_make_pc_index(instr);
        case 4:
            return operand_make_immediate_value(instr->size, instr);
        }
    default:
    {
        Operand* op = calloc(1, sizeof(Operand));
        op->instruction = instr;
        op->type = Unsupported;
        return op;
    }
    }
}

void operand_free(Operand* operand)
{
    free(operand);
}

int operand_tostring(Operand* operand, M68k* ctx, char* buffer)
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
    {
        int16_t displacement = m68k_fetch(ctx);
        uint32_t target = ctx->address_registers[operand->n] + displacement;
        return sprintf(buffer, "(%0X,A%d) [%010x]", displacement, operand->n, target);
    }
    case AddressRegisterIndirectIndexed:
        return sprintf(buffer, "TODO %d(A%d, D%d)", operand->n, operand->n, operand->n);
    case ProgramCounterDisplacement:
    {
        int16_t displacement = m68k_fetch(ctx);
        uint32_t target = ctx->pc + displacement - 2;
        return sprintf(buffer, "(%0X,PC) [%0X]", displacement, target);
    }
    case ProgramCounterIndexed:
        return sprintf(buffer, "TODO %d(PC, D%d)", operand->n, operand->n);
    case Immediate:
        return sprintf(buffer, "#$%0X", FETCH_EA_AND_GET(operand, ctx));
    case Value:
        return sprintf(buffer, "#$%0X", operand->n);
    case AbsoluteShort:
        return sprintf(buffer, "($%0X).w", FETCH_EA(operand, ctx));
    case AbsoluteLong:
        return sprintf(buffer, "($%0X).l", FETCH_EA(operand, ctx));
    case BranchingOffset:
    {
        int16_t offset = FETCH_EA_AND_GET(operand, ctx);
        uint32_t target = ctx->instruction_address + 2
            + (operand->instruction->size == Byte ? (int8_t)offset : offset);
        return sprintf(buffer, "$%0X [%0X]", offset, target);
    }
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
        return InvalidSize;
    }
}

Size operand_size2(uint8_t pattern)
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
        return InvalidSize;
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
        return InvalidSize;
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

void noop(Operand* o, M68k* ctx, uint32_t value)
{
}

// Most operands that point to memory data will use those to get/set values from the effective address

uint32_t get_from_ea(Operand* o, M68k* ctx)
{
    return m68k_read(ctx, o->instruction->size, o->last_ea);
}

void set_from_ea(Operand* o, M68k* ctx, uint32_t value)
{
    m68k_write(ctx, o->instruction->size, o->last_ea, value);
}

// Placeholder function for addressing modes that do not have effective address to compute

uint32_t fetch_no_ea(Operand* o, M68k* ctx)
{
    return 0;
}

/*
 * Data register
 */

uint32_t data_register_ea(Operand* o, M68k* ctx)
{
    return MASK_ABOVE_INC(ctx->data_registers[o->n], o->instruction->size);
}

uint32_t data_register_get(Operand* o, M68k* ctx)
{
    return MASK_ABOVE_INC(ctx->data_registers[o->n], o->instruction->size);
}

void data_register_set(Operand* o, M68k* ctx, uint32_t value)
{
    ctx->data_registers[o->n] =
        MASK_BELOW(ctx->data_registers[o->n], o->instruction->size) |
        MASK_ABOVE_INC(value, o->instruction->size);
}

Operand* operand_make_data_register(int n, Instruction* instr)
{
    Operand* op = calloc(1, sizeof(Operand));
    op->instruction = instr;
    op->type = DataRegister;
    op->fetch_ea_func = data_register_ea;
    op->get_value_func = data_register_get;
    op->set_value_func = data_register_set;
    op->n = n;
    return op;
}

/*
 * Address register
 */

uint32_t address_register_ea(Operand* o, M68k* ctx)
{
    return MASK_ABOVE_INC(ctx->address_registers[o->n], o->instruction->size);
}

uint32_t address_register_get(Operand* o, M68k* ctx)
{
    return MASK_ABOVE_INC(ctx->address_registers[o->n], o->instruction->size);
}

void address_register_set(Operand* o, M68k* ctx, uint32_t value)
{
    ctx->address_registers[o->n] =
        MASK_BELOW(ctx->address_registers[o->n], o->instruction->size) |
        MASK_ABOVE_INC(value, o->instruction->size);
}

Operand* operand_make_address_register(int n, Instruction* instr)
{
    Operand* op = calloc(1, sizeof(Operand));
    op->instruction = instr;
    op->type = AddressRegister;
    op->fetch_ea_func = address_register_ea;
    op->get_value_func = address_register_get;
    op->set_value_func = address_register_set;
    op->n = n;
    return op;
}

/*
 * Indirect address register
 *
 * The register contains the address of the data in memory.
 */

uint32_t address_indirect_ea(Operand* o, M68k* ctx)
{
    return ctx->address_registers[o->n] & 0xFFFFFF;
}

Operand* operand_make_address_register_indirect(int n, Instruction* instr)
{
    Operand* op = calloc(1, sizeof(Operand));
    op->instruction = instr;
    op->type = AddressRegisterIndirect;
    op->fetch_ea_func = address_indirect_ea;
    op->get_value_func = get_from_ea;
    op->set_value_func = set_from_ea;
    op->n = n;
    return op;
}

/*
 * Post-increment indirect address register
 *
 * The address register is incremented AFTER reading/writing data.
 * The increment value depends on the instruction size.
 */

void address_inc(Operand* o, M68k* ctx)
{
    ctx->address_registers[o->n] += size_in_bytes(o->instruction->size);
}

Operand* operand_make_address_register_indirect_postinc(int n, struct Instruction* instr)
{
    Operand* op = calloc(1, sizeof(Operand));
    op->instruction = instr;
    op->type = AddressRegisterIndirectPostInc;
    op->fetch_ea_func = address_indirect_ea;
    op->get_value_func = get_from_ea;
    op->set_value_func = set_from_ea;
    op->post_func = address_inc;
    op->n = n;
    return op;
}

/*
 * Pre-decrement indirect
 *
 * The address register is decremented BEFORE reading/writing data.
 * The decrement value depends on the instruction size.
 */

void address_dec(Operand* o, M68k* ctx)
{
    ctx->address_registers[o->n] -= size_in_bytes(o->instruction->size);
}

Operand* operand_make_address_register_indirect_predec(int n, struct Instruction* instr)
{
    Operand* op = calloc(1, sizeof(Operand));
    op->instruction = instr;
    op->type = AddressRegisterIndirectPreDec;
    op->fetch_ea_func = address_indirect_ea;
    op->get_value_func = get_from_ea;
    op->set_value_func = set_from_ea;
    op->pre_func = address_dec;
    op->n = n;
    return op;
}

/*
* Address indirect with displacement
*
* The data is located at the stored address + a displacement (extension)
*/

uint32_t address_indirect_displacement_ea(Operand* o, M68k* ctx)
{
    int16_t displacement = m68k_fetch(ctx);
    return  ctx->address_registers[o->n] + displacement;
}

Operand* operand_make_address_register_indirect_displacement(int n, struct Instruction* instr)
{
    Operand* op = calloc(1, sizeof(Operand));
    op->instruction = instr;
    op->type = AddressRegisterIndirectDisplacement;
    op->fetch_ea_func = address_indirect_displacement_ea;
    op->get_value_func = get_from_ea;
    op->set_value_func = set_from_ea;
    op->n = n;
    return op;
}

/*
* Address indirect with index
*/

uint32_t address_indirect_index_ea(Operand* o, M68k* ctx)
{
    M68k* m = ctx;
    uint32_t ext = m68k_fetch(m);

    uint32_t index = INDEX_LENGTH(ext) ? INDEX_REGISTER(ext) : SIGN_EXTEND_W(INDEX_REGISTER(ext));
    return m->address_registers[o->n] + (int8_t)INDEX_DISPLACEMENT(ext) + (int32_t)index;
}

Operand* operand_make_address_register_indirect_index(int n, struct Instruction* instr)
{
    Operand* op = calloc(1, sizeof(Operand));
    op->instruction = instr;
    op->type = AddressRegisterIndirectIndexed;
    op->fetch_ea_func = address_indirect_index_ea;
    op->get_value_func = get_from_ea;
    op->set_value_func = set_from_ea;
    op->n = n;
    return op;
}

/*
* Immediate value encoded in the extension words of an instruction
*/

uint32_t immediate_byte_word_ea(Operand* o, M68k* ctx)
{
    m68k_fetch(ctx);
    return ctx->pc - 2;
}

uint32_t immediate_long_ea(Operand* o, M68k* ctx)
{
    m68k_fetch(ctx);
    m68k_fetch(ctx);
    return ctx->pc - 4;
}

uint32_t immediate_byte_get(Operand* o, M68k* ctx)
{
    return MASK_ABOVE_INC(m68k_read_w(ctx, o->last_ea), 8);
}

uint32_t immediate_word_get(Operand* o, M68k* ctx)
{
    return m68k_read_w(ctx, o->last_ea);
}

uint32_t immediate_long_get(Operand* o, M68k* ctx)
{
    return m68k_read_l(ctx, o->last_ea);
}

Operand* operand_make_immediate_value(Size size, Instruction* instr)
{
    Operand* op = calloc(1, sizeof(Operand));
    op->instruction = instr;
    op->type = Immediate;

    switch (size) {
    case Byte:
        op->fetch_ea_func = immediate_byte_word_ea;
        op->get_value_func = immediate_byte_get;
        break;
    case Word:
        op->fetch_ea_func = immediate_byte_word_ea;
        op->get_value_func = immediate_word_get;
        break;
    case Long:
        op->fetch_ea_func = immediate_long_ea;
        op->get_value_func = immediate_long_get;
        break;
    }

    return op;
}

/*
 * Address encoded in the extension words of an instruction
 */

uint32_t absolute_short_ea(Operand* o, M68k* ctx)
{
    uint16_t address = m68k_fetch(ctx);
    return SIGN_EXTEND_W(address);
}

Operand* operand_make_absolute_short(Instruction* instr)
{
    Operand* op = calloc(1, sizeof(Operand));
    op->instruction = instr;
    op->type = AbsoluteShort;
    op->fetch_ea_func = absolute_short_ea;
    op->get_value_func = get_from_ea;
    op->set_value_func = set_from_ea;
    return op;
}

uint32_t absolute_long_ea(Operand* o, M68k* ctx)
{
    return m68k_fetch(ctx) << 16 | m68k_fetch(ctx);
    // TODO not sure about w.l
}

Operand* operand_make_absolute_long(Instruction* instr)
{
    Operand* op = calloc(1, sizeof(Operand));
    op->instruction = instr;
    op->type = AbsoluteLong;
    op->fetch_ea_func = absolute_long_ea;
    op->get_value_func = get_from_ea;
    op->set_value_func = set_from_ea;
    return op;
}

/*
 *
 */

uint32_t pc_displacement_word_ea(Operand* o, M68k* ctx)
{
    int16_t displacement = m68k_fetch(ctx);
    return ctx->instruction_address + 2 + displacement;
}

Operand* operand_make_pc_displacement(Instruction* instr)
{
    Operand* op = calloc(1, sizeof(Operand));
    op->type = ProgramCounterDisplacement;
    op->instruction = instr;
    op->fetch_ea_func = pc_displacement_word_ea;
    op->get_value_func = get_from_ea;
    op->set_value_func = set_from_ea;
    return op;
}

/*
* Program Counter with index
*
* The data is located at the current program counter + a displacement + the value of an index register (extensions)
*
* https://github.com/traviscross/libzrtp/blob/master/third_party/bnlib/lbn68000.c#L342
*/

uint32_t pc_index_ea(Operand* o, M68k* ctx)
{
    M68k* m = ctx;
    uint32_t ext = m68k_fetch(m);

    uint32_t index = INDEX_LENGTH(ext) ? INDEX_REGISTER(ext) : SIGN_EXTEND_W(INDEX_REGISTER(ext));
    return m->instruction_address + 2 + (int8_t)INDEX_DISPLACEMENT(ext) + (int32_t)index;
}

Operand* operand_make_pc_index(struct Instruction* instr)
{
    Operand* op = calloc(1, sizeof(Operand));
    op->instruction = instr;
    op->type = ProgramCounterIndexed;
    op->fetch_ea_func = pc_index_ea;
    op->get_value_func = get_from_ea;
    op->set_value_func = set_from_ea;
    return op;
}

/*
* Value directly stored within the opcode
*/

uint32_t value_get(Operand* o, M68k* ctx)
{
    return o->n;
}

Operand* operand_make_value(int value, struct Instruction* instr)
{
    Operand* op = calloc(1, sizeof(Operand));
    op->type = Value;
    op->instruction = instr;
    op->n = value;
    op->fetch_ea_func = fetch_no_ea;
    op->get_value_func = value_get;
    return op;
}

/*
 * Branching offset, specific to control flow instruction such as Bcc
 */

uint32_t branching_offset_ea(Operand* o, M68k* ctx)
{
    m68k_fetch(ctx);
    return ctx->pc - 2;
}

uint32_t branching_offset_byte_get(Operand* o, M68k* ctx)
{
    return ctx->instruction_register & 0xFF;
}

uint32_t branching_offset_word_get(Operand* o, M68k* ctx)
{
    return m68k_read_w(ctx, ctx->instruction_address + 2);
}

Operand* operand_make_branching_offset(Instruction* instr, Size size)
{
    Operand* op = calloc(1, sizeof(Operand));
    op->instruction = instr;
    op->type = BranchingOffset;

    op->set_value_func = noop;

    switch (size)
    {
    case Byte:
        op->fetch_ea_func = fetch_no_ea; // The offset is in the low byte of the opcode, no need to advance the PC
        op->get_value_func = branching_offset_byte_get;
        break;
    case Word:
        op->fetch_ea_func = branching_offset_ea;
        op->get_value_func = branching_offset_word_get;
        break;
    default: // The 68000 does not support long branching offsets
        free(op);
        return NULL;
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
        if (operand->fetch_ea_func == immediate_byte_word_ea)
            return 2;
        return 4;
    case BranchingOffset:
        return operand->instruction->size == Byte ? 0 : 2;
    default:
        return 0;
    }
}

#define FATAL(...) do { fprintf(stderr, "FATAL(in %s): ", __func__); fprintf(stderr, __VA_ARGS__); exit(1); } while (0)

uint32_t operand_fetch_ea_and_get(Operand* operand, M68k* ctx) {
    if (operand->fetch_ea_func == NULL) {
        FATAL("fetch_ea_func is null (opcode: %04X, name: %s)\n",
              operand->instruction->opcode, operand->instruction->name);
    }
    operand->last_ea = operand->fetch_ea_func(operand, ctx);
    if (operand->get_value_func == NULL) {
        FATAL("get_value_func is null (opcode: %04X, name: %s)\n",
              operand->instruction->opcode, operand->instruction->name);
    }
    return operand->get_value_func(operand, ctx);
}

void operand_fetch_ea_and_set(Operand* operand, M68k* ctx, uint32_t value) {
    if (operand->fetch_ea_func == NULL) {
        FATAL("fetch_ea_func is null (opcode: %04X, name: %s)\n",
              operand->instruction->opcode, operand->instruction->name);
    }
    operand->last_ea = operand->fetch_ea_func(operand, ctx);
    if (operand->set_value_func == NULL) {
        FATAL("set_value_func is null (opcode: %04X, name: %s)\n",
              operand->instruction->opcode, operand->instruction->name);
    }
    operand->set_value_func(operand, ctx, value);
}
