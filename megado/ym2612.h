#pragma once

#include <stdbool.h>
#include <stdint.h>

// YM2612 - Main sound chip
// http://md.squee.co/YM2612
// http://www.smspower.org/maxim/Documents/YM2612

typedef struct Operator {
    uint8_t detune                       : 3;
    uint8_t multiple                     : 4;
    uint8_t total_level                  : 7;
    uint8_t rate_scaling                 : 2;
    uint8_t attack_rate                  : 5;
    bool    amplitude_modulation_enabled : 1;
    uint8_t first_decay_rate             : 5;
    uint8_t second_decay_rate            : 5;
    uint8_t second_amplitude             : 4;
    uint8_t release_rate                 : 4;
    // uint8_t ssg_eg                    : 4;  // proprietary register, skipping
} Operator;

typedef struct Frequency {
    uint8_t  block     : 3;
    uint16_t freq      : 11;
} Frequency;

typedef struct Channel {
    Operator operators[4];

    Frequency frequency;
    uint8_t   feedback                         : 3;
    uint8_t   algorithm                        : 3;
    bool      left_output                      : 1;
    bool      right_output                     : 1;
    uint8_t   amplitude_modulation_sensitivity : 2;
    uint8_t   frequency_modulation_sensitivity : 3;
} Channel;

typedef struct YM2612
{
    uint8_t  latched_address_part1;
    uint8_t  latched_address_part2;

    bool     lfo_enabled         : 1;
    uint8_t  lfo_frequency_index : 3;
    uint16_t timer_a             : 10;
    uint8_t  timer_b             : 8;
    uint8_t  channel3_mode       : 2;
    uint8_t  channel6_mode       : 2;
    uint8_t  dac_data            : 8;
    bool     dac_enabled         : 1;

    Channel channels[6];

    // These are only used by channels 3 & 6, but it's easier to keep them in
    // the global struct:
    Frequency channel3_additional_frequencies[3];
    Frequency channel6_additional_frequencies[3];

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

void ym2612_initialize(YM2612*);
uint8_t ym2612_read(YM2612*, uint32_t address);
void ym2612_write(YM2612*, uint32_t address, uint8_t value);
void ym2612_run_cycles(YM2612*, uint16_t);
