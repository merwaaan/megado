#pragma once

#include <stdint.h>

struct Operand;

typedef uint16_t (*GetFunc)(struct Operand* this);
typedef void (*SetFunc)(struct Operand* this, uint16_t value);

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

char* operand_tostring(Operand operand);

Operand operand_make_data_register(int n);
uint16_t data_register_get(Operand* this);
void data_register_set(Operand* this, uint16_t value);

Operand operand_make_address_register(int n);
uint16_t address_register_get(Operand* this);
void address_register_set(Operand* this, uint16_t value);

Operand operand_make_address(int n);
uint16_t address_get(Operand* this);
void address_set(Operand* this, uint16_t value);
