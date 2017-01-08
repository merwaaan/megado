#pragma once

#include <stdint.h>

#include "bit_utils.h"

#define GET(operand) (operand)->get((operand))
#define SET(operand, value) (operand)->set((operand), (value))

struct Operand;
struct Instruction;

typedef uint32_t(*GetFunc)(struct Operand* this);
typedef void(*SetFunc)(struct Operand* this, uint32_t value);
typedef void(*Action)(struct Operand* this);

typedef enum {
    Unsupported,
    Data,
    Address,
    AddressIndirect,
    AddressIndirectPreInc,
    AddressIndirectPostInc,
    AddressIndirectDisplacement,
    AddressIndirectIndexed,
    ProgramCounterDisplacement,
    ProgramCounterIndexed,
    Immediate,
    AbsoluteShort,
    AbsoluteLong,
    Value,
    Condition
} OperandType;

typedef struct Operand {
    OperandType type;

    GetFunc get;
    SetFunc set;
    
    Action pre;
    Action post;

    int n;

    struct Instruction* instruction;
} Operand;

int operand_tostring(Operand* operand, char* buffer);

Operand* operand_make_data(int n, struct Instruction*); // TODO put instr first to be consistent with the order modules
Operand* operand_make_address(int n, struct Instruction*);
Operand* operand_make_address_indirect(int n, struct Instruction*);
Operand* operand_make_address_indirect_postincrement(int n, struct Instruction*);
Operand* operand_make_address_indirect_predecrement(int n, struct Instruction*);
Operand* operand_make_address_indirect_displacement(int n, struct Instruction*);
Operand* operand_make_immediate(Size, struct Instruction*);
Operand* operand_make_absolute_short(struct Instruction*);
Operand* operand_make_absolute_long(struct Instruction*);
Operand* operand_make_pc_displacement(struct Instruction*);
Operand* operand_make_value(int value, struct Instruction*);
Operand* operand_make_condition(int pattern, struct Instruction*);

Size operand_size(uint8_t pattern);
Size operand_size2(uint8_t pattern);
Size operand_size3(uint8_t pattern); // TODO how to name this?

uint8_t operand_sign_extension(uint8_t pattern);

Operand* operand_make(uint16_t pattern, struct Instruction* instr);
void operand_free(Operand* instr);
