#pragma once

#include <stdint.h>

#include "bit_utils.h"

struct Operand;
struct Instruction;

// Read/Write functions for each type of operand
//
// The address of the instance of the instruction that the operand is part of
// must be passed in order to handle context-dependent operands (eg. extension
// words). In most cases, this address will be the current PC.
typedef uint32_t(*GetFunc)(struct Operand* o, uint32_t instr_address);
typedef void(*SetFunc)(struct Operand* o, uint32_t instr_address, uint32_t value);

// Shortcuts to quickly read/write operand data
// -- Relative to a given address
#define GET_RELATIVE(operand, instr_address) (operand)->get((operand), (instr_address)) 
#define SET_RELATIVE(operand, instr_address, value) (operand)->set((operand), (instr_address), (value))
// -- Relative to the current PC
#define GET(operand) (operand)->get((operand), (operand)->instruction->context->pc) 
#define SET(operand, value) (operand)->set((operand), (operand)->instruction->context->pc, (value))

typedef void(*Action)(struct Operand* this);

typedef enum
{
    Unsupported,
    DataRegister,
    AddressRegister,
    AddressRegisterIndirect,
    AddressRegisterIndirectPreDec,
    AddressRegisterIndirectPostInc,
    AddressRegisterIndirectDisplacement,
    AddressRegisterIndirectIndexed,
    ProgramCounterDisplacement,
    ProgramCounterIndexed,
    Immediate,
    AbsoluteByte,
    AbsoluteShort,
    AbsoluteLong,
    Value,

    BranchingOffset
} OperandType;

typedef struct Operand
{
    // The instruction that the operand instance is bound to.
    // The operand data depends on the instruction size.
    struct Instruction* instruction;

    OperandType type;

    GetFunc get;
    SetFunc set;
    
    Action pre;
    Action post;

    int n;
} Operand;

int operand_tostring(Operand* operand, uint32_t instr_address, char* buffer);

Operand* operand_make_data_register(int n, struct Instruction*); // TODO put instr first to be consistent with the order modules
Operand* operand_make_address_register(int n, struct Instruction*);
Operand* operand_make_address_register_indirect(int n, struct Instruction*);
Operand* operand_make_address_register_indirect_predec(int n, struct Instruction*);
Operand* operand_make_address_register_indirect_postinc(int n, struct Instruction*);
Operand* operand_make_address_register_indirect_displacement(int n, struct Instruction*);
Operand* operand_make_immediate_value(Size, struct Instruction*);
Operand* operand_make_absolute_short(struct Instruction*);
Operand* operand_make_absolute_long(struct Instruction*);
Operand* operand_make_pc_displacement(struct Instruction*);
Operand* operand_make_value(int value, struct Instruction*);
Operand* operand_make_branching_offset(struct Instruction*, Size size);

Size operand_size(uint8_t pattern);
Size operand_size2(uint8_t pattern);
Size operand_size3(uint8_t pattern); // TODO how to name this?

uint8_t operand_sign_extension(uint8_t pattern);

int operand_length(Operand*);

Operand* operand_make(uint16_t pattern, struct Instruction* instr);
void operand_free(Operand* instr);
