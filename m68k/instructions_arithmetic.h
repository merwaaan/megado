#pragma once

#include "m68k.h"
#include "operands.h"

#define DEFINE_INSTR(NAME) Instruction* gen_ ## NAME (uint16_t opcode, M68k* context)

DEFINE_INSTR(clr);
DEFINE_INSTR(ext);
DEFINE_INSTR(mulu);
DEFINE_INSTR(muls);
