#include <stdio.h>
#include <stdlib.h>

#include "ym2612.h"

YM2612* ym2612_make() {
  return calloc(1, sizeof(YM2612));
}

void ym2612_free(YM2612* y) {
  free(y);
}

uint8_t ym2612_read(YM2612* y, uint32_t address) {
  // BUSY is the highest bit
  // lower two bits are for timer overflows
  // But the timer feature is seemingly not used by many games
  return 0;
}

void ym2612_write(YM2612* y, uint32_t address, uint8_t value) {
}
