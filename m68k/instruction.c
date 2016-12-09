#include <stdlib.h>

#include "instruction.h"
#include "m68k.h"
#include "operands.h"

bool instruction_valid(Instruction* instr)
{
    // Check that the generated instruction is valid
    if (instr == NULL)
        return false;

    // Check that the instruction's operands are valid
    for (int o = 0; o < instr->operand_count; ++o)
        if (instr->operands[o] == NULL)
            return false;

    return true;
}

Instruction* instruction_make(M68k* context, char* name, InstructionFunc func)
{
    Instruction* i = calloc(1, sizeof(Instruction));
    i->context = context;
    i->name = name;
    i->func = func;
    return i;
}

void instruction_free(Instruction* instr)
{
    if (instr == NULL)
        return;

    for (int i = 0; i < instr->operand_count; ++i)
        operand_free(instr->operands[i]);

    free(instr);
}
