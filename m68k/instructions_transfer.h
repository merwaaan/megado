#pragma once

#include "m68k.h"
#include "operands.h"

#define DEFINE_INSTR(NAME) Instruction* gen_ ## NAME (uint16_t opcode, M68k* context)

DEFINE_INSTR(exg);
DEFINE_INSTR(lea);
DEFINE_INSTR(move);
DEFINE_INSTR(move);
DEFINE_INSTR(movea);
DEFINE_INSTR(movem);
DEFINE_INSTR(moveq);
DEFINE_INSTR(movep);
DEFINE_INSTR(pea);
