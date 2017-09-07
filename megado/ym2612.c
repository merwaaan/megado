#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "m68k/bit_utils.h"
#include "audio.h"
#include "genesis.h"
#include "ym2612.h"

// TODO: output audio directly instead of writing to WAV
// TODO: envelope
// TODO: stereo
// TODO: feedback for channel 1
// TODO: channel 3 & 6 special modes
// TODO: modulation
// TODO: timers
// TODO: dac?

#ifdef DEBUG
#define LOG_YM2612(...) printf(__VA_ARGS__)
#else
#define LOG_YM2612(...)
#endif

const uint32_t YM2612_NTSC_FREQUENCY = 7670453;
const uint16_t MASTER_CYCLES_PER_ENVELOPE_CLOCK = 351;
const uint16_t MASTER_CYCLES_PER_YM2612_CLOCK = 144;
// NTSC_FREQUENCY / MASTER_CYCLES_PER_CLOCK / SAMPLE_RATE
//        7670453 /                     144 /       44100
const double YM2612_CLOCKS_PER_SAMPLE = 1.20786926808;

// Local functions

void channel_clock(Channel*);
void envelope_clock(YM2612*);

// Global functions

YM2612* ym2612_make(Genesis* g) {
  YM2612* y = calloc(1, sizeof(YM2612));
  y->genesis = g;
  return y;
}

void ym2612_free(YM2612* y) {
  free(y);
}

void ym2612_initialize(YM2612* y) {
    // Reset but save Genesis pointer
    Genesis* g = y->genesis;
    memset(y, 0, sizeof(YM2612));
    y->genesis = g;
}

void ym2612_clock(YM2612* y) {
    for (int i=0; i < 6; ++i) {
        channel_clock(&y->channels[i]);
    }
}

void ym2612_run_cycles(YM2612* y, uint16_t cycles) {
    y->remaining_clocks += cycles;

    while (y->remaining_clocks > 0) {
        ym2612_clock(y);
        y->remaining_clocks -= MASTER_CYCLES_PER_YM2612_CLOCK;

        y->envelope_remaining_clocks += MASTER_CYCLES_PER_YM2612_CLOCK;
        while (y->envelope_remaining_clocks > 0) {
            envelope_clock(y);
            y->envelope_remaining_clocks -= MASTER_CYCLES_PER_ENVELOPE_CLOCK;
        }

        y->sample_counter++;
        while (y->sample_counter > 0) {
            ym2612_emit_sample_cb(y, ym2612_mix(y));
            y->sample_counter -= YM2612_CLOCKS_PER_SAMPLE;
        }
    }
}

int16_t ym2612_mix(YM2612* y) {
    int32_t sample = 0;

    for (int i=0; i < 6; ++i) {
        sample += channel_output(&y->channels[i]);
    }
    sample /= 6;

    return sample;
}

void envelope_clock(YM2612* y) {
    // TODO:
}

uint8_t detune_table[32][4] = {
    {0, 0, 1, 2},   //0  (0x00)
    {0, 0, 1, 2},   //1  (0x01)
    {0, 0, 1, 2},   //2  (0x02)
    {0, 0, 1, 2},   //3  (0x03)
    {0, 1, 2, 2},   //4  (0x04)
    {0, 1, 2, 3},   //5  (0x05)
    {0, 1, 2, 3},   //6  (0x06)
    {0, 1, 2, 3},   //7  (0x07)
    {0, 1, 2, 4},   //8  (0x08)
    {0, 1, 3, 4},   //9  (0x09)
    {0, 1, 3, 4},   //10 (0x0A)
    {0, 1, 3, 5},   //11 (0x0B)
    {0, 2, 4, 5},   //12 (0x0C)
    {0, 2, 4, 6},   //13 (0x0D)
    {0, 2, 4, 6},   //14 (0x0E)
    {0, 2, 5, 7},   //15 (0x0F)
    {0, 2, 5, 8},   //16 (0x10)
    {0, 3, 6, 8},   //17 (0x11)
    {0, 3, 6, 9},   //18 (0x12)
    {0, 3, 7,10},   //19 (0x13)
    {0, 4, 8,11},   //20 (0x14)
    {0, 4, 8,12},   //21 (0x15)
    {0, 4, 9,13},   //22 (0x16)
    {0, 5,10,14},   //23 (0x17)
    {0, 5,11,16},   //24 (0x18)
    {0, 6,12,17},   //25 (0x19)
    {0, 6,13,19},   //26 (0x1A)
    {0, 7,14,20},   //27 (0x1B)
    {0, 8,16,22},   //28 (0x1C)
    {0, 8,16,22},   //29 (0x1D)
    {0, 8,16,22},   //30 (0x1E)
    {0, 8,16,22}    //31 (0x1F)
};

void channel_clock(Channel* c) {
    for (int i=0; i < 4; ++i) {
        uint32_t phase_increment = c->frequency.freq;

        // Shift by block
        if (c->frequency.block > 1) {
            phase_increment <<= c->frequency.block;
        } else if (c->frequency.block == 0) {
            phase_increment >>= 1;
        }

        uint16_t f = c->frequency.freq;
        uint8_t n4 = BIT(f, 10);
        uint8_t n3 = (BIT(f, 10) & (BIT(f, 9) | BIT(f, 8) | BIT(f, 7)))
            | !(BIT(f, 10) & BIT(f, 9) & BIT(f, 8) & BIT(f, 7));

        uint8_t key_code = (c->frequency.block << 2) | (n4 << 1) | n3;
        uint8_t detune_adjust = detune_table[key_code][c->operators[i].detune & 0x3];

        // MSB of detune is the sign bit
        if (c->operators[i].detune & 0x4) {
            phase_increment -= detune_adjust;
        } else {
            phase_increment += detune_adjust;
        }

        // Multiply by multiple
        if (c->operators[i].multiple > 0) {
            phase_increment *= c->operators[i].multiple;
        } else {
            phase_increment >>= 1;
        }

        c->operators[i].phase_counter += phase_increment;
    }
}

double operator_phase(Operator* o) {
    return ((double)(o->phase_counter >> 10)) / ((1 << 10) - 1);
}

#define TAU 6.283185307178586

int16_t channel_output(Channel* c) {
    // FIXME: use something faster than sin()
    if (c->enabled) {
        double output = 0;
        switch (c->algorithm) {
        case 0: {
            double s1 = sin(TAU * operator_phase(&c->operators[0]));
            double s2 = sin(s1 + TAU * operator_phase(&c->operators[1]));
            double s3 = sin(s2 + TAU * operator_phase(&c->operators[2]));
            double s4 = sin(s3 + TAU * operator_phase(&c->operators[3]));
            output = s4;
        } break;

        case 1: {
            double s1 = sin(TAU * operator_phase(&c->operators[0]));
            double s2 = sin(TAU * operator_phase(&c->operators[1]));
            double s3 = sin(s1 + s2 + TAU * operator_phase(&c->operators[2]));
            double s4 = sin(s3 + TAU * operator_phase(&c->operators[3]));
            output = s4;
        } break;

        case 2: {
            double s1 = sin(TAU * operator_phase(&c->operators[0]));
            double s2 = sin(TAU * operator_phase(&c->operators[1]));
            double s3 = sin(s2 + TAU * operator_phase(&c->operators[2]));
            double s4 = sin(s1 + s3 + TAU * operator_phase(&c->operators[3]));
            output = s4;
        } break;

        case 3: {
            double s1 = sin(TAU * operator_phase(&c->operators[0]));
            double s2 = sin(s1 + TAU * operator_phase(&c->operators[1]));
            double s3 = sin(TAU * operator_phase(&c->operators[2]));
            double s4 = sin(s2 + s3 + TAU * operator_phase(&c->operators[3]));
            output = s4;
        } break;

        case 4: {
            double s1 = sin(TAU * operator_phase(&c->operators[0]));
            double s2 = sin(s1 + TAU * operator_phase(&c->operators[1]));
            double s3 = sin(TAU * operator_phase(&c->operators[2]));
            double s4 = sin(s3 + TAU * operator_phase(&c->operators[3]));
            output = (s2 + s4) / 2;
        } break;

        case 5: {
            double s1 = sin(TAU * operator_phase(&c->operators[0]));
            double s2 = sin(s1 + TAU * operator_phase(&c->operators[1]));
            double s3 = sin(s1 + TAU * operator_phase(&c->operators[2]));
            double s4 = sin(s1 + TAU * operator_phase(&c->operators[3]));
            output = (s2 + s3 + s4) / 3;
        } break;

        case 6: {
            double s1 = sin(TAU * operator_phase(&c->operators[0]));
            double s2 = sin(s1 + TAU * operator_phase(&c->operators[1]));
            double s3 = sin(TAU * operator_phase(&c->operators[2]));
            double s4 = sin(TAU * operator_phase(&c->operators[3]));
            output = (s2 + s3 + s4) / 3;
        } break;

        case 7:{
            double s1 = sin(TAU * operator_phase(&c->operators[0]));
            double s2 = sin(TAU * operator_phase(&c->operators[1]));
            double s3 = sin(TAU * operator_phase(&c->operators[2]));
            double s4 = sin(TAU * operator_phase(&c->operators[3]));
            output = (s1 + s2 + s3 + s4) / 4;
        } break;

        }

        return channel_envelope(c) * output;
    } else {
        return 0;
    }
}

float channel_frequency_in_hertz(Channel* c) {
    if (c->frequency.block > 0) {
        return ((float) (c->frequency.freq << (c->frequency.block - 1)))
            * ((float) YM2612_NTSC_FREQUENCY) / 1048576.0f / 144.0f;
    } else {
        // FIXME: handle block 0
        return 0;
    }
}

int16_t channel_envelope(Channel* c) {
    // TODO: envelope
    return 16422;
}

// TEMP: move to a proper audio backend
#define YM2612_MAX_SAMPLES 4096
int16_t ym2612_samples[YM2612_MAX_SAMPLES];
uint32_t ym2612_samples_cursor = 0;

void ym2612_emit_sample_cb(YM2612* y, int16_t sample) {
    ym2612_samples[ym2612_samples_cursor++] = sample;

    if (ym2612_samples_cursor == YM2612_MAX_SAMPLES) {
        if (SDL_QueueAudio(y->genesis->audio->device, ym2612_samples, sizeof(ym2612_samples)) != 0) {
            fprintf(stderr, "Failed to queue audio: %s", SDL_GetError());
        }
        ym2612_samples_cursor = 0;
    }
}

uint8_t ym2612_read(YM2612* y, uint32_t address) {
  // BUSY is the highest bit
  // lower two bits are for timer overflows
  // But the timer feature is seemingly not used by many games


  // Fake it for now: always return non-busy and timer overflowed.
  return 0x7;
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
        uint8_t channel   =  value       & 0x7;

        if (channel == 3 || channel == 7) {
            printf("Warning: YM2612 key on for invalid channel: %d\n", channel);
            break;
        }

        if (channel > 2) {
            channel--;
        }

        if (operators == 0xf) {
            y->channels[channel].enabled = true;
        } else if (operators == 0) {
            y->channels[channel].enabled = false;
        } else {
            // FIXME: operators can be enabled invidually, but
            // does it mean they do not output any frequency otherwise?
            printf("Warning: YM2612 key on for individual operators: %x\n", operators);
        }

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

        // Operators 2 and 3 are swapped
        if (op == 1) { op = 2; }
        else if (op == 2) { op = 1; }

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
            channels[chan].operators[op].decay_rate                   = value;
            break;

        case 0x70:
            channels[chan].operators[op].sustain_rate = value;
            break;

        case 0x80:
            channels[chan].operators[op].sustain_level    = value >> 4;
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
        // FIXME: japanese manual has different mappings
        additional_freqs[op].freq = (additional_freqs[op].freq & 0x700) | value;
    } break;

    case 0xac: case 0xad: case 0xae: {
        uint8_t op = 1 + (address & 3);
        // FIXME: japanese manual has different mappings
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
        printf("Warning: unhandled write to YM2612 (%s): %x <- %x\n",
               part2 ? "part II" : "part I", address, value);
    }
}

void ym2612_write(YM2612* y, uint32_t address, uint8_t value) {
    LOG_YM2612("Write to YM2612: %x %x\n", address, value);

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
