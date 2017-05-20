#include <stdlib.h>

#include "cycles.h"
#include "instruction.h"
#include "m68k.h"
#include "operands.h"

Instruction* gen_abcd(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "ABCD", not_implemented);
    return i;
}

Instruction* gen_nbcd(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "NBCD", not_implemented);
    return i;
}

Instruction* gen_sbcd(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "SBCD", not_implemented);
    return i;
}
