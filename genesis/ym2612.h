#pragma once

#include <stdbool.h>
#include <stdint.h>

// YM2612 - Main sound chip
// http://md.squee.co/YM2612

typedef struct YM2612
{
    // TODO what does LFO mean?
    bool lfo_enabled;
    uint8_t lfo_frequency_index;

    uint16_t timer_a;
    uint16_t timer_b;

    uint8_t dac_value;
    bool dac_enabled;
} YM2612;

static float lfo_frequencies[] = {
    3.98,
    5.56,
    6.02,
    6.37,
    6.88,
    9.63,
    48.1,
    72.2
};

YM2612* ym2612_make();
void ym2612_free(YM2612*);

uint8_t ym2612_read(YM2612*, uint32_t address);
void ym2612_write(YM2612*, uint32_t address, uint8_t value);
