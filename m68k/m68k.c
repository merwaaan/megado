#include <stdbool.h>
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
    {0xC000, 0xF000, &gen_and},
    {0x4A00, 0xFF00, &gen_tst},
    {0x4600, 0xFF00, &gen_not},
    //{0x5000, 0xF000, &gen_scc},
    {0x8000, 0xF000, &gen_or},
    {0xB000, 0xF000, &gen_eor},
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

Instruction* generate(uint16_t opcode, M68k* context)
{
    int pattern_count = sizeof(_patterns) / sizeof(Pattern);

    for (int i = 0; i < pattern_count; ++i)
        if (pattern_match(opcode, _patterns[i]))
            return pattern_generate(_patterns[i], opcode, context);

    return NULL;
}

void execute(uint16_t opcode)
{

}

M68k* m68k_init()
{
    M68k* m68k = calloc(1, sizeof(M68k));

    m68k->opcode_table = calloc(0x10000, sizeof(Instruction*));

    // Generate every possible opcode
    for (int opcode = 0; opcode < 0x10000; opcode++)
    {
        Instruction* instr = generate(opcode, m68k);

        if (!instruction_valid(instr))
        {
            instruction_free(instr);
            continue;
        }

        m68k->opcode_table[opcode] = instr;

        printf("opcode %#04X: %s", opcode, instr->name);
        for (int i = 0; i < instr->operand_count; ++i)
            printf(" %s", operand_tostring(instr->operands[i]));
        printf("\n");
    }

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

    instr->func(instr);
}
