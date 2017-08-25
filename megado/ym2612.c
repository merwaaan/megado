#include <stdio.h>
#include <stdlib.h>

#include "ym2612.h"

// Local functions
void ym2612_write_register(YM2612*, uint8_t, uint8_t, bool);

YM2612* ym2612_make() {
  return calloc(1, sizeof(YM2612));
}

void ym2612_free(YM2612* y) {
  free(y);
}

void ym2612_initialize(YM2612* y) {

}

void ym2612_run_cycles(YM2612* y, uint16_t cycles) {

}

uint8_t ym2612_read(YM2612* y, uint32_t address) {
  // BUSY is the highest bit
  // lower two bits are for timer overflows
  // But the timer feature is seemingly not used by many games


  // Fake it for now: always return non-busy and timer overflowed.
  return 0x7;
}

void ym2612_write(YM2612* y, uint32_t address, uint8_t value) {
    printf("Write to YM2612: %x %x\n", address, value);

    switch (address) {
        // Part I: channels 1, 2 and 3
    case 0x4000:
        y->latched_address_part1 = value;
        break;

    case 0x4001:
        ym2612_write_register(y, y->latched_address_part1, value, false);
        break;

        // Part II: channels 4, 5 and 6
    case 0x4002:
        y->latched_address_part2 = value;
        break;
    case 0x4003:
        ym2612_write_register(y, y->latched_address_part2, value, true);
        break;

    default:
        printf("Warning: unhandled write in YM2612: %x <- %x\n", address, value);
    }
}

void ym2612_write_register(YM2612* y, uint8_t address, uint8_t value, bool part2) {
    Channel* channels = y->channels;
    Frequency* additional_freqs = y->channel3_additional_frequencies;
    if (part2) {
        // Write to channels 4, 5 and 6 instead
        channels = &y->channels[3];
        additional_freqs = y->channel6_additional_frequencies;
    }

    switch (address) {
    case 0x22:
        y->lfo_enabled         = value >> 3;
        y->lfo_frequency_index = value;
        break;

    case 0x24:
        y->timer_a = (y->timer_a & 0x300) | value;
        break;
    case 0x25:
        y->timer_a = (y->timer_a & 0x0ff) | (((uint16_t) value) << 8);
        break;

    case 0x26:
        y->timer_b = value;
        break;

    case 0x27:
        if (part2) {
            y->channel6_mode = value >> 6;
        } else {
            y->channel3_mode = value >> 6;
        }

        // TODO: timers load/enable/reset
        break;

    case 0x28: {
        uint8_t operators = (value >> 4) & 0xf;
        uint8_t channel   =  value       & 0xf;

        // TODO: key on for that channel & operators
    } break;

    case 0x2a:
        y->dac_data = value;
        break;

    case 0x2b:
        y->dac_enabled = value >> 7;
        break;

    case 0x30 ... 0x9e: {
        uint8_t op    = (address >> 2) & 3;
        uint8_t chan  =  address       & 3;

        // Ignore invalid channels or operators
        if (op == 3 || chan == 3) {
            break;
        }

        switch (address & 0xf0) {
        case 0x30:
            channels[chan].operators[op].detune   = value >> 4;
            channels[chan].operators[op].multiple = value;
            break;

        case 0x40:
            channels[chan].operators[op].total_level = value;
            break;

        case 0x50:
            channels[chan].operators[op].rate_scaling = value >> 6;
            channels[chan].operators[op].attack_rate  = value;
            break;

        case 0x60:
            channels[chan].operators[op].amplitude_modulation_enabled = value >> 7;
            channels[chan].operators[op].first_decay_rate             = value;
            break;

        case 0x70:
            channels[chan].operators[op].second_decay_rate = value;
            break;

        case 0x80:
            channels[chan].operators[op].second_amplitude = value >> 4;
            channels[chan].operators[op].release_rate     = value;
            break;

        case 0x90:
            // proprietary register, skipping
            break;
        }
    } break;

    case 0xa0 ... 0xa2: {
        uint8_t chan = address & 3;
        channels[chan].frequency.freq = (channels[chan].frequency.freq & 0x700) | value;
    } break;

    case 0xa4 ... 0xa6: {
        uint8_t chan = address & 3;
        channels[chan].frequency.block = value >> 3;
        channels[chan].frequency.freq  = (channels[chan].frequency.freq & 0x0ff) | (((uint16_t) value) << 8);
    } break;

    case 0xa8 ... 0xaa: {
        uint8_t op = address & 3;
        additional_freqs[op].freq = (additional_freqs[op].freq & 0x700) | value;
    } break;

    case 0xac ... 0xae: {
        uint8_t op = 1 + (address & 3);
        additional_freqs[op].block = value >> 3;
        additional_freqs[op].freq = (additional_freqs[op].freq & 0x0ff) | (((uint16_t) value) << 8);
    } break;

    case 0xb0 ... 0xb2: {
        uint8_t chan = address & 3;
        channels[chan].feedback  = value >> 3;
        channels[chan].algorithm = value;
    } break;

    case 0xb4 ... 0xb6: {
        uint8_t chan = address & 3;
        channels[chan].left_output                      = value >> 7;
        channels[chan].right_output                     = value >> 6;
        channels[chan].amplitude_modulation_sensitivity = value >> 3;
        channels[chan].frequency_modulation_sensitivity = value;
    } break;

    default:
        printf("Warning: unhandled write to YM2612 Part I: %x <- %x\n", address, value);
    }
}
