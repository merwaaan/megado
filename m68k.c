#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "m68k.h"
#include "operands.h"

Instruction *_instructions;

void not(Operand *operands)
{
	//dst.set(dst.get() + src.get());

	// TODO flags
}

uint16_t fragment(int x, int start, int end)
{
	return (x & ~(0xFFFF << start)) >> end;
}

Operand make_operand(uint16_t pattern)
{
	switch (pattern & 0x38)
	{
	case 0:
		return operand_make_data_register(pattern & 7);
	case 0x8:
		return operand_make_address_register(pattern & 7);
	case 0x10:
		return operand_make_address(pattern & 7);
	default:
		return (Operand) {};
	}
}

Instruction gen_not(uint16_t opcode)
{
	Operand ops[] = {
		make_operand(fragment(opcode, 5, 0))
	};

  return (Instruction) {
	"NOT",
    not,
    ops,
    1
  };
}

static Pattern _patterns[] =
{
	{0x4600, 0xFF00, &gen_not}
};

int pattern_match(uint16_t opcode, Pattern pattern)
{
  return (opcode & pattern.mask) == pattern.pattern;
}

Instruction pattern_generate(uint16_t opcode, Pattern pattern)
{
  return pattern.generator(opcode);
}

char* instruction_tostring(Instruction instr)
{
	char buffer[1024];
	int pos = 0;

	pos += sprintf(buffer, "%s", instr.name);

	for (int i = 0; i < instr.operandCount; ++i)
		pos += sprintf(buffer + pos, " %s", operand_tostring(instr.operands[i]));

	return buffer;
}

Instruction generate(uint16_t opcode)
{
  for (int i = 0; i < 1; ++i)
    if (pattern_match(opcode, _patterns[i]))
    {
      Instruction instr = pattern_generate(opcode, _patterns[i]);
      printf("opcode %#04X: %s\n", opcode, instruction_tostring(instr));
      return instr;
    }

    //printf("Opcode %d does not match any pattern\n", opcode);
    return (Instruction) {};
}

void execute(uint16_t opcode)
{
	Instruction instr = _instructions[opcode];
	instr.func(instr.operands);
}

void m68k_init()
{
	_instructions = calloc(0x10000, sizeof(Instruction));

	for (int i = 0; i < 0x10000; i++)
		_instructions[i] = generate(i);
}
