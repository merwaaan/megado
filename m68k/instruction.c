#include "instruction.h"
#include "operands.h"

#include <stdlib.h>

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

void instruction_free(Instruction* instr)
{
    for (int i = 0; i < instr->operand_count; ++i)
        operand_free(instr->operands[i]);

    free(instr);
}
