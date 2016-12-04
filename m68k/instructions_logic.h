#pragma once

#include "globals.h"
#include "instructions_logic.h"
#include "m68k.h"
#include "operands.h"

#define Z(x) _m68k.flags = _m68k.flags & (0xFFFF ^ 1 << 2)
#define V(x) _m68k.flags = _m68k.flags & (0xFFFF ^ 1 << 1)
#define C(x) _m68k.flags = _m68k.flags & (0xFFFF ^ 1)

#define DEFINE_INSTR(NAME) Instruction* gen_ ## NAME (uint16_t opcode, M68k* context);

DEFINE_INSTR(and)
DEFINE_INSTR(eor)
DEFINE_INSTR(or)
