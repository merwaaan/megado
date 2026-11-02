#pragma once

#include <stdint.h>

#include "bit_utils.h"

struct Operand;
struct Instruction;

/*
 * Macros to read/write operand data
 */

// Fetch and store the operand's effective address
#define FETCH_EA(OPERAND, CTX) (OPERAND)->last_ea = (OPERAND)->fetch_ea_func((OPERAND), (CTX))

// Get/Set the operand's value, the stored effective address will be used
#define GET(OPERAND, CTX) (OPERAND)->get_value_func((OPERAND), (CTX))
#define SET(OPERAND, CTX, VALUE) (OPERAND)->set_value_func((OPERAND), (CTX), (VALUE))

// Fetch the operand's effective address and then get/set its value, in one call
#define FETCH_EA_AND_GET(OPERAND, CTX) (operand_fetch_ea_and_get((OPERAND), (CTX)))
#define FETCH_EA_AND_SET(OPERAND, CTX, VALUE) (operand_fetch_ea_and_set((OPERAND), (CTX), (VALUE)))

struct M68k;

// Effective address resolution function
//
// Calling such a function might trigger a data fetch and advance the program counter
// so it must not be done more than once per instruction call.
typedef uint32_t (*FetchEAFunc)(struct Operand*, struct M68k*);

// Read/Write functions to access an operand's data after its effective address has been computed
//
// Can be called as many times as necessary.
typedef uint32_t (*GetValueFunc)(struct Operand*, struct M68k*);
typedef void (*SetValueFunc)(struct Operand*, struct M68k*, uint32_t value);

// Pre/Post instruction action
typedef void (*Action)(struct Operand*, struct M68k*);

typedef enum
{
    DataRegister,
    AddressRegister,
    AddressRegisterIndirect,
    AddressRegisterIndirectPostInc,
    AddressRegisterIndirectPreDec,
    AddressRegisterIndirectDisplacement,
    AddressRegisterIndirectIndexed,
    AbsoluteShort,
    AbsoluteLong,
    ProgramCounterDisplacement,
    ProgramCounterIndexed,
    Immediate,

    Value, // TODO unclear, rename to Quick?
    BranchingOffset,

    // Operands are stored by value in instructions, so this is used to represent inexistant operands
    NoOperand
} OperandType;

typedef struct Operand
{
    // The instruction that the operand instance is bound to
    struct Instruction* instruction;

    OperandType type;

    // Last effective address computed
    uint32_t last_ea;

    // Compute and store the operand's effective address
    FetchEAFunc fetch_ea_func;

    // Get/Set the operand's value
    GetValueFunc get_value_func;
    SetValueFunc set_value_func;

    Action pre_func;
    Action post_func;

    int n;
} Operand;

int operand_tostring(Operand* operand, struct M68k*, char* buffer);

void operand_make_data_register(Operand*, int n, struct Instruction*); 
void operand_make_address_register(Operand*, int n, struct Instruction*);
void operand_make_address_register_indirect(Operand*, int n, struct Instruction*);
void operand_make_address_register_indirect_predec(Operand*, int n, struct Instruction*);
void operand_make_address_register_indirect_postinc(Operand*, int n, struct Instruction*);
void operand_make_address_register_indirect_displacement(Operand*, int n, struct Instruction*);
void operand_make_address_register_indirect_index(Operand*, int n, struct Instruction*);
void operand_make_immediate_value(Operand*, Size, struct Instruction*);
void operand_make_absolute_short(Operand*, struct Instruction*);
void operand_make_absolute_long(Operand*, struct Instruction*);
void operand_make_pc_displacement(Operand*, struct Instruction*);
void operand_make_pc_index(Operand*, struct Instruction*);
void operand_make_value(Operand*, int value, struct Instruction*);
void operand_make_branching_offset(Operand*, struct Instruction*, Size size);

Size operand_size(uint8_t pattern);
Size operand_size2(uint8_t pattern);
Size operand_size3(uint8_t pattern); // TODO how to name this?

uint8_t operand_sign_extension(uint8_t pattern);

int operand_length(Operand*);

void operand_make(Operand*, uint16_t pattern, struct Instruction* instr);

uint32_t operand_fetch_ea_and_get(Operand*, struct M68k*);
void operand_fetch_ea_and_set(Operand*, struct M68k*, uint32_t);
