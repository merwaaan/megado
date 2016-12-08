#include <stdbool.h>
#include <stdlib.h>

#include "bit_utils.h"
#include "instruction.h"
#include "instructions_logic.h"
#include "m68k.h"
#include "operands.h"

void jmp(Instruction* i)
{
    i->context->pc = GET(i->operands[0]);
}

Instruction* gen_jmp(uint16_t opcode, M68k* m)
{
    Instruction* i = calloc(1, sizeof(Instruction));
    i->context = m;
    i->name = "JMP";
    i->func = jmp;

    i->operands = calloc(2, sizeof(Operand*));
    i->operands[1] = operand_make(FRAGMENT(opcode, 5, 0), i);
    i->operand_count = 1;

    return i;
}
