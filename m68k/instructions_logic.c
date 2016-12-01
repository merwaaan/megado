#include "instructions_logic.h"
#include "operands.h"

void and(Operand *operands)
{
    uint16_t r = GET(operands[0]) & GET(operands[1]);
    SET(operands[1], r);

    // TODO flags
}

Instruction gen_and(uint16_t opcode, M68k* m)
{
    Operand op1 = operand_make_data_register(FRAGMENT(opcode, 12, 10), m);
    Operand op2 = make_operand(FRAGMENT(opcode, 5, 0), m);
    int direction = BIT(opcode, 8);

    Operand operands[2];
    operands[direction] = op1;
    operands[direction + 1 % 2] = op2;

	return (Instruction) {"AND", and, operands, 2};
}

/*
void eor(Operand *operands)
{
	//operands[1].set(operands[0].get() ^ operands[1].get());

	// TODO flags
}

Instruction gen_eor(uint16_t opcode)
{
	Operand ops[] = {
		make_operand(fragment(opcode, 5, 0))
	};

	return (Instruction) { "EOR", eor, ops, 1 };
}

void or(Operand *operands)
{
	//operands[1].set(operands[0].get() | operands[1].get());

	// TODO flags
}

Instruction gen_or(uint16_t opcode)
{
	Operand ops[] = {
		make_operand(fragment(opcode, 5, 0))
	};

	return (Instruction) { "OR", or, ops, 1 };
}

void not(Operand *operands)
{
	//operands[0].set(~operands[0].get());

	// TODO flags
}

Instruction gen_not(uint16_t opcode)
{
	Operand ops[] = {
		make_operand(fragment(opcode, 5, 0))
	};

	return (Instruction) { "NOT", not, ops, 1 };
}

void scc(Operand *operands)
{
	// TODO set value wrt cond
	// TODO flags
}

Instruction gen_scc(uint16_t opcode)
{
	//ConditionFunc cond = make_condition(fragment(opcode, 11, 8));

	Operand ops[] = {
		make_operand(fragment(opcode, 5, 0))
	};

	return (Instruction) { "SCC", scc, ops, 1 };
}

void tst(Operand *operands)
{
	// TODO flags
}

Instruction gen_tst(uint16_t opcode)
{
	Operand ops[] = {
		make_operand(fragment(opcode, 5, 0))
	};

	return (Instruction) { "TST", tst, ops, 1 };
}
*/
