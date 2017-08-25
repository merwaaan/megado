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

        // MSVC does not support the `...` operator in case
    case 0x30: case 0x31: case 0x32: case 0x34: case 0x35: case 0x36:
    case 0x38: case 0x39: case 0x3a: case 0x3c: case 0x3d: case 0x3e:
    case 0x40: case 0x41: case 0x42: case 0x44: case 0x45: case 0x46:
    case 0x48: case 0x49: case 0x4a: case 0x4c: case 0x4d: case 0x4e:
    case 0x50: case 0x51: case 0x52: case 0x54: case 0x55: case 0x56:
    case 0x58: case 0x59: case 0x5a: case 0x5c: case 0x5d: case 0x5e:
    case 0x60: case 0x61: case 0x62: case 0x64: case 0x65: case 0x66:
    case 0x68: case 0x69: case 0x6a: case 0x6c: case 0x6d: case 0x6e:
    case 0x70: case 0x71: case 0x72: case 0x74: case 0x75: case 0x76:
    case 0x78: case 0x79: case 0x7a: case 0x7c: case 0x7d: case 0x7e:
    case 0x80: case 0x81: case 0x82: case 0x84: case 0x85: case 0x86:
    case 0x88: case 0x89: case 0x8a: case 0x8c: case 0x8d: case 0x8e:
    case 0x90: case 0x91: case 0x92: case 0x94: case 0x95: case 0x96:
    case 0x98: case 0x99: case 0x9a: case 0x9c: case 0x9d: case 0x9e: {
        uint8_t op    = (address >> 2) & 3;
        uint8_t chan  =  address       & 3;

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

    case 0xa0: case 0xa1: case 0xa2: {
        uint8_t chan = address & 3;
        channels[chan].frequency.freq = (channels[chan].frequency.freq & 0x700) | value;
    } break;

    case 0xa4: case 0xa5: case 0xa6: {
        uint8_t chan = address & 3;
        channels[chan].frequency.block = value >> 3;
        channels[chan].frequency.freq  = (channels[chan].frequency.freq & 0x0ff) | (((uint16_t) value) << 8);
    } break;

    case 0xa8: case 0xa9: case 0xaa: {
        uint8_t op = address & 3;
        additional_freqs[op].freq = (additional_freqs[op].freq & 0x700) | value;
    } break;

    case 0xac: case 0xad: case 0xae: {
        uint8_t op = 1 + (address & 3);
        additional_freqs[op].block = value >> 3;
        additional_freqs[op].freq = (additional_freqs[op].freq & 0x0ff) | (((uint16_t) value) << 8);
    } break;

    case 0xb0: case 0xb1: case 0xb2: {
        uint8_t chan = address & 3;
        channels[chan].feedback  = value >> 3;
        channels[chan].algorithm = value;
    } break;

    case 0xb4: case 0xb5: case 0xb6: {
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
