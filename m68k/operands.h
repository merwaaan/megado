#pragma once

#include <stdint.h>

struct Operand;

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
} Operand;

Operand make_operand(uint16_t pattern);
char* operand_tostring(Operand operand);

Operand operand_make_data_register(int n);
Operand operand_make_address_register(int n);
Operand operand_make_address(int n);
