#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "m68k/bit_utils.h"
#include "genesis.h"
#include "ym2612.h"

// TODO: envelope
// TODO: stereo
// TODO: feedback for channel 1
// TODO: channel 3 & 6 special modes
// TODO: CSM
// TODO: modulation
// TODO: timers
// TODO: dac?

#ifdef DEBUG
#define LOG_YM2612(...) printf(__VA_ARGS__)
#else
#define LOG_YM2612(...)
#endif

// The YM2612 divides the master clock by 7
// Then, the FM clock divides the YM2612 clock by 144
static const uint32_t MASTER_CYCLES_PER_FM_CLOCK = 1008; // 144 * 7

// The envelope clock divides the FM clock by 3
static const uint16_t MASTER_CYCLES_PER_ENVELOPE_CLOCK = 336; // 1008 / 3

// Local functions

static void ym2612_update(YM2612 *y);

// Global functions

YM2612* ym2612_make(Genesis* g) {
  YM2612* y = calloc(1, sizeof(YM2612));
  y->genesis = g;
  return y;
}

void ym2612_free(YM2612* y) {
    free(y);
}

static int16_t accm[24][2];
static int sample[2];
static uint8_t cycles;
static int frame_buffer[1080 * 2 * 24];
static int *buffer;
static blip_t *blips[3];

void ym2612_initialize(YM2612* y) {

    // Reset but save Genesis pointer
    Genesis* g = y->genesis;
    memset(y, 0, sizeof(YM2612));
    y->genesis = g;
    OPN2_Reset(&y->ym3438);

    cycles = 0;
    buffer = frame_buffer;
}

static void ym2612_update(YM2612 *y) {
    while (y->remaining_master_cycles > 0) {
        OPN2_Clock(&y->ym3438, accm[cycles]);
        y->remaining_master_cycles -= 6;

        cycles = (cycles + 1) % 24;
        if (cycles == 0) {
            sample[0] = 0;
            sample[1] = 0;
            for (uint8_t i=0; i < 24; ++i) {
                sample[0] += accm[i][0];
                sample[1] += accm[i][1];
            }
        }

        *buffer++ = sample[0] * 8;
        *buffer++ = sample[1] * 8;
    }
}

void ym2612_run_cycles(YM2612* y, uint32_t cycles) {
    // Ignore these cycles since we already got them from m68k_run_cycles
    //y->remaining_master_cycles += cycles;

    ym2612_update(y);
}

int16_t ym2612_mix(YM2612* y) {
    int *ptr = frame_buffer;
    int preamp = 2;
    int fm_cycles;

    int prev_l = 0;
    int prev_r = 0;

    while (fm_cycles > 0) {
        int l = (*ptr++ * preamp) / 100;
        int r = (*ptr++ * preamp) / 100;
        blip_add_delta(blips[0], time, l - prev_l, r - prev_r);
        prev_l = l;
        prev_r = r;

        fm_cycles -= 6 * 7;
    }

    blip_end_frame(blips[0], cycles);

    return blip_samples_avail(blips[0]);
}

uint8_t ym2612_read(YM2612* y, uint32_t address) {
    return OPN2_Read(&y->ym3438, address);
}



void ym2612_write(YM2612* y, uint32_t address, uint8_t value) {
    // Catch up with the M68K

    ym2612_update(y);

    LOG_YM2612("Write to YM2612: %x %x\n", address, value);
    OPN2_Write(&y->ym3438, address, value);
}
