#include <stdbool.h>
#include <stdint.h>

typedef struct Z80 {
  uint32_t left_cycles;

  uint16_t pc;

  bool running; // Whether the Z80 is executing instructions

  bool bus_req; // 0: request bus, 1: release bus
  bool bus_ack; // 0: bus is free, 1: Z80 is using the bus
  bool reset;   // 0: reset the Z80, 1: let the Z80 run
} Z80;

Z80* z80_make();
void z80_free(Z80*);

void z80_initialize(Z80*);

uint8_t z80_step(Z80*);
void z80_run_cycles(Z80*, int);

uint8_t z80_busreq_r(Z80*);
void z80_busreq_w(Z80*, uint8_t);
