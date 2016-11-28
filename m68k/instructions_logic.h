#pragma once

void and(Operand *operands)
{
	operands[1].set(operands[0].get() & operands[1].get());

	// TODO flags
}

Instruction gen_and(uint16_t opcode)
{
	Operand ops[] = {
		make_operand(fragment(opcode, 5, 0))
	};

	return (Instruction) { "AND", and, ops, 1 };
}

void eor(Operand *operands)
{
	operands[1].set(operands[0].get() ^ operands[1].get());

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
	operands[1].set(operands[0].get() | operands[1].get());

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
	operands[0].set(~operands[0].get());

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
	ConditionFunc cond = make_condition(fragment(opcode, 11, 8));

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
