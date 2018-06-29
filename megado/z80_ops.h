#pragma once

#include <stdint.h>

#include "z80.h"

typedef uint8_t(*z80_op)(Z80*);

uint8_t z80_op_nop(Z80 *z);

#include "z80_op_func_gen.h"

static z80_op z80_op_table[] = {
#include "z80_op_table_gen.h"
};

static DecodedZ80Instruction z80_disasm_table[] = {
#include "z80_disasm_table_gen.h"
};
