#pragma once

#include <stdint.h>

#define GET(operand) operand->get(operand)
#define SET(operand, value) operand->set(operand, value)

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
    AddressIndirectOffset,
    AddressIndirectIndexOffset,
    ProgramCounterOffset,
    ProgramCounterIndexOffset,
    Immediate,
    AbsoluteShort,
    AbsoluteLong,
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

Operand* operand_make_data(int n, struct Instruction* instr);
Operand* operand_make_address(int n, struct Instruction* instr);
Operand* operand_make_address_indirect(int n, struct Instruction* instr);
Operand* operand_make_address_indirect_postincrement(int n, struct Instruction* instr);
Operand* operand_make_address_indirect_predecrement(int n, struct Instruction* instr);
Operand* operand_make_immediate(int n, struct Instruction* instr);
Operand* operand_make_absolute_short(struct Instruction* instr);
Operand* operand_make_absolute_long(struct Instruction* instr);
Operand* operand_make_condition(int pattern, struct Instruction* instr);

Operand* operand_make(uint16_t pattern, struct Instruction* instr);

void operand_free(Operand* instr);

uint8_t operand_size(uint8_t pattern);
uint8_t operand_size2(uint8_t pattern); // TODO how to name this?