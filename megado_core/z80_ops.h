#pragma once

#include <stdint.h>

#include "z80.h"

typedef uint8_t (*z80_op)(Z80*);

uint8_t z80_op_nop(Z80* z);

extern z80_op z80_op_table[256];

static DecodedZ80Instruction z80_disasm_table[] = {
#include "z80_disasm_table_gen.h"
};
