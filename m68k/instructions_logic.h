#pragma once

#include "m68k.h"
#include "operands.h"

#define DEFINE_INSTR(NAME) Instruction* gen_ ## NAME (uint16_t opcode, M68k* context)

DEFINE_INSTR(and);
DEFINE_INSTR(andi);
DEFINE_INSTR(eor);
DEFINE_INSTR(eori);
DEFINE_INSTR(or);
DEFINE_INSTR(ori);
DEFINE_INSTR(not);
//DEFINE_INSTR(scc);
DEFINE_INSTR(tst);
