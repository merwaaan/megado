#pragma once

#include <stdint.h>

#define GET(operand) (operand.get(operand))
#define SET(operand, value) (operand.set(operand, value))

struct Operand;
struct Instruction;

typedef uint16_t (*GetFunc)(struct Operand this);
typedef void (*SetFunc)(struct Operand this, uint16_t value);

typedef enum {
	Unsupported,
	DataRegister,
	AddressRegister,
	Address
} OperandType;

typedef struct Operand {
	OperandType type;

	GetFunc get;
	SetFunc set;

	int n;

	struct Instruction* instruction;
} Operand;

char* operand_tostring(Operand operand);

Operand make_operand(uint16_t pattern, struct Instruction* instr);
Operand operand_make_data_register(int n, struct Instruction* instr);
//Operand operand_make_address_register(int n);
//Operand operand_make_address(int n);
