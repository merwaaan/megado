#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "bit_utils.h"
#include "instruction.h"
#include "instructions_logic.h"
#include "m68k.h"
#include "operands.h"

static Pattern _patterns[] =
{
    {0xC000, 0xF000, &gen_and}
	/*{0x4A00, 0xFF00, &gen_tst},
	{0x4600, 0xFF00, &gen_not},
	{0x5000, 0xF000, &gen_scc},
	{0x8000, 0xF000, &gen_or},
	{0xB000, 0xF000, &gen_eor},*/
	//{0xC000, 0xF000, &gen_add},
};

int pattern_match(uint16_t opcode, Pattern pattern)
{
	return (opcode & pattern.mask) == pattern.pattern;
}

Instruction* pattern_generate(Pattern pattern, uint16_t opcode, M68k* context)
{
	return pattern.generator(opcode, context);
}

/*char* instruction_tostring(Instruction instr)
{
	char buffer[1024];
	int pos = 0;

	pos += sprintf(buffer, "%s", instr.name);

	for (int i = 0; i < instr.operand_count; ++i)
		pos += sprintf(buffer + pos, " %s", operand_tostring(instr.operands[i]));

	return buffer;
}*/

Instruction* generate(uint16_t opcode, M68k* context)
{
	for (int i = 0; i < 1; ++i)
		if (pattern_match(opcode, _patterns[i]))
		{
			Instruction* instr = pattern_generate(_patterns[i], opcode, context);

            Operand o1 = instr->operands[0];
            Operand o2 = instr->operands[1];

            printf("opcode %#04X: %s", opcode, instr->name);
            for (int i = 0; i < instr->operand_count; ++i)
                printf(" %s", operand_tostring(instr->operands + i));
            printf("\n");

          return instr;
		}

	//printf("Opcode %d does not match any pattern\n", opcode);
	return NULL;
}

void execute(uint16_t opcode)
{

}

M68k* m68k_init()
{
    M68k* m68k = malloc(sizeof(M68k));

	// Generate every possible opcode
	m68k->opcode_table = calloc(0x10000, sizeof(Instruction*));

	int and = parse_bin("1100 011 0 01 000101");
	m68k->opcode_table[and] = generate(and, m68k);
	//for (int i = 0; i < 0x10000; i++)
	//	m68k->opcode_table[i] = generate(i, m68k);

    return m68k;
}

void m68k_free(M68k* cpu)
{
	free(cpu);
}

void m68k_execute(M68k* cpu, uint16_t opcode)
{
    Instruction* instr = cpu->opcode_table[opcode];
    if (instr == NULL)
    {
        printf("Opcode %#08X cannot be found in the opcode table\n", opcode);
        return;
    }

	instr->func(instr->operands);
}
