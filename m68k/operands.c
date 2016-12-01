#include <stdio.h>

#include "operands.h"

#include "globals.h"
#include "m68k.h"

char* operand_tostring(Operand operand)
{
	char buffer[1024];

	switch (operand.type)
	{
	case DataRegister:
		sprintf(buffer, "D%d", operand.n);
		break;
	case AddressRegister:
		sprintf(buffer, "A%d", operand.n);
		break;
	case Address:
		sprintf(buffer, "(A%d)", operand.n);
		break;
	default:
		sprintf(buffer, "UNSUPPORTED");
	}

	return buffer;
}


Operand make_operand(uint16_t pattern, struct M68k* context)
{
	switch (pattern & 0x38)
	{
	case 0:
		return operand_make_data_register(pattern & 7, context);
	/*case 0x8:
		return operand_make_address_register(pattern & 7);
	case 0x10:
		return operand_make_address(pattern & 7);*/
	default:
		return (Operand) {};
	}
}

/*
 *
 */

uint16_t data_register_get(Operand this)
{
	return this.context->data_registers[this.n];
}

void data_register_set(Operand this, uint16_t value)
{
	this.context->data_registers[this.n] = value;
}

Operand operand_make_data_register(int n, M68k* context)
{
	return (Operand) {
		.type = DataRegister,
		.get = data_register_get,
		.set = data_register_set,
		.context = context,
		.n = n
	};
}

/*
 *
 */

/*uint16_t address_register_get(Operand this)
{
	return _m68k.address_registers[this.n];
}

void address_register_set(Operand this, uint16_t value)
{
	_m68k.address_registers[this.n] = value;
}

Operand operand_make_address_register(int n)
{
	return (Operand) {
		.type = AddressRegister,
		.get = address_register_get,
		.set = address_register_set,
		.n = n
	};
}*/

/*
 *
 */

/*uint16_t address_get(Operand this)
{
	return _memory[_m68k.address_registers[this.n]];
}

void address_set(Operand this, uint16_t value)
{
	_memory[_m68k.address_registers[this.n]] = value;
}

Operand operand_make_address(int n)
{
	return (Operand) {
		.type = Address,
		.get = address_get,
		.set = address_set,
		.n = n
	};
}
*/
