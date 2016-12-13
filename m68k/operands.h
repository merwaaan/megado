#pragma once

#include <stdint.h>

#define GET(operand) operand->get(operand)
#define SET(operand, value) operand->set(operand, value)

struct Operand;
struct Instruction;

typedef int32_t (*GetFunc)(struct Operand* this);
typedef void (*SetFunc)(struct Operand* this, int32_t value);

typedef enum {
	Unsupported,
	DataRegister,
	AddressRegister,
	AddressRegisterIndirect,
    Immediate,
    Extension,
    Condition
} OperandType;

typedef struct Operand {
	OperandType type;

	GetFunc get;
	SetFunc set;

	int n;

	struct Instruction* instruction;
} Operand;

char* operand_tostring(Operand* operand);

Operand* operand_make_data_register(int n, struct Instruction* instr);
Operand* operand_make_address_register(int n, struct Instruction* instr);
Operand* operand_make_address_register_indirect(int n, struct Instruction* instr);
Operand* operand_make_address_register_indirect_postincrement(int n, struct Instruction* instr); // TODO
Operand* operand_make_address_register_indirect_predecrement(int n, struct Instruction* instr); // TODO
Operand* operand_make_immediate(int n, struct Instruction* instr);
Operand* operand_make_extension(int length, struct Instruction* instr);
Operand* operand_make_condition(int pattern, struct Instruction* instr);

Operand* operand_make(uint16_t pattern, struct Instruction* instr);

void operand_free(Operand* instr);

uint8_t operand_size(uint8_t pattern);