#pragma once

#include <stdint.h>
#include <stdio.h>

struct GYM {
  uint8_t *data;
  uint64_t size;

  struct GYMHeader {
    char tag[4];
    char song[32];
    char game[32];
    char copyright[32];
    char emulator[32];
    char dumper[32];
    char comment[256];
    uint32_t loop_start;
    uint32_t packed_size;
  } *header;
};

void gym_play(struct GYM gym);
void gym_stop();
struct GYM read_gym(FILE *gym_file);
