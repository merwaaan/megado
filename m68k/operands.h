#pragma once

#include <stdint.h>

#define GET(operand) (operand->get(operand))
#define SET(operand, value) (operand->set(operand, value))

struct Operand;
struct Instruction;

typedef int32_t (*GetFunc)(struct Operand* this);
typedef void (*SetFunc)(struct Operand* this, int32_t value);

typedef enum {
	Unsupported,
	DataRegister,
	AddressRegister,
	AddressRegisterIndirect
} OperandType;

typedef struct Operand {
	OperandType type;

	GetFunc get;
	SetFunc set;

	int n;

	struct Instruction* instruction;
} Operand;

char* operand_tostring(Operand* operand);

Operand* operand_make(uint16_t pattern, struct Instruction* instr);
Operand* operand_make_data_register(int n, struct Instruction* instr);
Operand* operand_make_address_register(int n, Instruction* instr);
Operand* operand_make_address_register_indirect(int n, Instruction* instr);

void operand_free(Operand* instr);

uint8_t operand_size(uint8_t pattern);