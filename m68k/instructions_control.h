#pragma once

#include "m68k.h"
#include "operands.h"

#define DEFINE_INSTR(NAME) Instruction* gen_ ## NAME (uint16_t opcode, M68k* context)

DEFINE_INSTR(bcc);
// TODO DEFINE_INSTR(dbcc);
DEFINE_INSTR(bra);
DEFINE_INSTR(bsr);
DEFINE_INSTR(jmp);
DEFINE_INSTR(jsr);
//DEFINE_INSTR(rtd);
//DEFINE_INSTR(rtr);
DEFINE_INSTR(rts);
