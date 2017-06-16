#include <stdio.h>
#include <stdlib.h>

#include "z80.h"
#include "z80_ops.h"

#ifdef DEBUG
#define LOG_Z80(...) printf(__VA_ARGS__)
#else
#define LOG_Z80(...)
#endif

Z80* z80_make() {
  Z80* z80 = calloc(1, sizeof(Z80));
  return z80;
}

void z80_free(Z80* z) {
  free(z);
}

void z80_initialize(Z80* z) {
  z->left_cycles = 0;

  // Charles McDonald doc says the Z80 is running on reset, but Sonic expects
  // otherwise.
  z->pc = 0;
  z->running = 0;
}

uint8_t z80_step(Z80* z) {
  uint16_t opcode = z->ram[z->pc++];

  z80_op op = z80_op_table[opcode];

  if (op == NULL) {
    LOG_Z80("z80: Unknown opcode: %02x\n", opcode);
    return 4;
  } else {
    LOG_Z80("z80: %04x: %02x\n", z->pc, opcode);
    // TODO: check this is a simple `call *rax` even with the static ops table
    return (*op)(z);
  }

}

void z80_run_cycles(Z80* z, int cycles) {
  if (z->resetting || !z->running) return;

  z->left_cycles += cycles;

  while (z->left_cycles > 0) {
    z->left_cycles -= z80_step(z);
  }
}

void z80_reset(Z80* z, uint8_t rst) {
  if (rst == 0) {
    z->pc = 0;
    z->left_cycles = 0;
    z->resetting = true;
    LOG_Z80("z80: RESET ON\n");
  } else {
    z->resetting = false;
    LOG_Z80("z80: RESET OFF\n");
  }
}

void z80_bus_req(Z80 *z, uint8_t req) {
  if (req) {
    z->running = 0;
    LOG_Z80("z80: BUSREQ ON\n");
  } else {
    z->running = 1;
    LOG_Z80("z80: BUSREQ OFF\n");
  }
}

uint8_t z80_bus_ack(Z80 *z) {
  // Can't write to the Z80 memory until it has stopped running
  if (z->running) {
    return 1;
    LOG_Z80("z80: BUSACK BUSY\n");
  } else {
    return 0;
    LOG_Z80("z80: BUSACK FREE\n");
  }
}

uint8_t z80_read(Z80 *z, uint16_t address) {
  LOG_Z80("z80: READ @ %04x\n", address);

  // Z80 RAM
  if (address < 0x2000) {
    return z->ram[address];
  }

  // RAM mirror
  else if (address < 0x4000) {
    return z->ram[address - 0x2000];

  }

  switch (address) {
    // YM2612
  case 0x4000:
  case 0x4001:
  case 0x4002:
  case 0x4003:
    // Fake it for now: always return non-busy and timer overflowed.
    return 0x7;

  default:
    return 0xFF;
  }
}

void z80_write(Z80 *z, uint16_t address, uint8_t value) {
  LOG_Z80("z80: WRITE @ %04x : %02x\n", address, value);

  // Z80 RAM
  if (address < 0x2000) {
    z->ram[address] = value;
  }

  // RAM mirror
  else if (address < 0x4000) {
    z->ram[address - 0x2000] = value;
  }
}

uint8_t z80_op_nop(Z80 *z) {
  return 4;
}
