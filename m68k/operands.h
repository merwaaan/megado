#pragma once

#include <stdint.h>

#include "bit_utils.h"

struct Operand;
struct Instruction;

// Effective address resolution function
//
// Calling such a function might trigger a data fetch so it must not be done
// more than once per instruction call.
typedef uint32_t(*FetchEAFunc)(struct Operand* o);

// Read/Write functions to access an operand's data from its effective address.
typedef uint32_t(*GetValueFunc)(struct Operand* o, uint32_t ea);
typedef void(*SetValueFunc)(struct Operand* o, uint32_t ea, uint32_t value);

/*
 * Macros to read/write operand data
 */

// Fetch and store the operand's effective address
#define FETCH_EAE(operand) (operand)->last_ea = (operand)->fetch_ea_func(operand) 

// Get/Set the operand's value, the stored effective address will be used
#define GETE(operand) (operand)->get_value_func((operand), (operand)->last_ea) 
#define SETE(operand, value) (operand)->set_value_func((operand), (operand)->last_ea, (value))

// Fetch the operand's effective address and then get/set its value
#define FETCH_EA_AND_GETE(operand) (operand)->get_value_func((operand), (operand)->last_ea = (operand)->fetch_ea_func(operand)) 
#define FETCH_EA_AND_SETE(operand, value) (operand)->set_value_func((operand), (operand)->last_ea = (operand)->fetch_ea_func(operand), (value))

// Pre/Post instruction action
typedef void(*Action)(struct Operand* this);

typedef enum
{
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
    AbsoluteShort,
    AbsoluteLong,
    Value,

    BranchingOffset,
    Unsupported
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

// Return the number of cycles required to compute an effective address
int operand_get_cycles(Operand*);

Operand* operand_make(uint16_t pattern, struct Instruction* instr);
void operand_free(Operand* instr);
