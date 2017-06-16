#pragma once

#include <stdint.h>

#include "z80.h"

typedef uint8_t(*z80_op)(Z80*);

uint8_t z80_op_nop(Z80 *z);

static z80_op z80_op_table[0x100] = {
  [0] = z80_op_nop,
};
