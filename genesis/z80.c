#include <stdio.h>
#include <stdlib.h>

#include "z80.h"

Z80* z80_make() {
  Z80* z80 = calloc(1, sizeof(Z80));
  return z80;
}

void z80_free(Z80* z) {
  free(z);
}

void z80_initialize(Z80* z) {
  z->left_cycles = 0;

  //  On power-up, the Z80 will be reset and will have the bus.
  z->bus_req = 0;
  z->bus_ack = 1;

  z->reset = 1;
  z->pc = 0;
  z->running = 1;
}

uint8_t z80_step(Z80* z) {
  // Request for bus, relinquish for the next cycle
  if (z->bus_req) {
    z->running = 0;
    z->bus_ack = 0;
  } else {
    // Bus is released, retrieve lines for next cycle
    z->running = 1;
    z->bus_ack = 1;
  }

  // Reset requested
  if (z->reset == 0) {
    z->pc = 0;
  }

  // NOP
  return 4;
}

void z80_run_cycles(Z80* z, int cycles) {
  z->left_cycles += cycles;

  while (z->left_cycles > 0) {
    z->left_cycles -= z80_step(z);
  }
}
