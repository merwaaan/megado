#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "ym3438.h"

// YM2612 - Main sound chip
// http://md.squee.co/YM2612
// http://www.smspower.org/maxim/Documents/YM2612

typedef struct YM2612
{
    struct Genesis* genesis;
    int32_t remaining_master_cycles;
    ym3438_t ym3438;
} YM2612;

YM2612* ym2612_make(struct Genesis*);
void ym2612_free(YM2612*);

void ym2612_initialize(YM2612*);
uint8_t ym2612_read(YM2612*, uint32_t address);
void ym2612_write(YM2612*, uint32_t address, uint8_t value);
void ym2612_run_cycles(YM2612*, uint32_t);
int16_t ym2612_mix(YM2612*);
