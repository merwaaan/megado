#pragma once

#include <stdint.h>

// PSG: Programmable Sound Generator
// see: http://www.smspower.org/Development/SN76489

struct Genesis;

typedef struct SquareChannel {
    uint8_t volume : 4;
    uint16_t tone : 10;
    uint16_t counter : 10;
    uint8_t output : 1;
} SquareChannel;

typedef struct NoiseChannel {
    uint8_t volume : 4;
    union {
        uint8_t noise;
        struct {
            uint8_t mode : 1;
            uint8_t shift_rate : 2;
        };
    };
    uint16_t counter : 10;
} NoiseChannel;

typedef struct PSG {
    uint16_t remaining_clocks;
    float sample_counter;

    SquareChannel square[3];
    NoiseChannel noise;

    uint8_t latched_channel : 2;
    uint8_t latched_register : 1;
} PSG;

PSG* psg_make(struct Genesis*);
void psg_free(PSG*);
void psg_write(PSG*, uint8_t);
void psg_clock(PSG*);
void psg_run_cycles(PSG*, uint16_t);
int16_t psg_mix(PSG*);

// Callback function called by psg_clock whenever a sample is ready
void psg_emit_sample_cb(uint16_t);

// TEMP: write to WAV for testing
#include <stdbool.h>
bool wav_write(const char* filename);
