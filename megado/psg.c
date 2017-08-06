#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "psg.h"

void write_latch(PSG*, uint8_t);
void write_data(PSG*, uint8_t);
void square_clock_frequency(SquareChannel*);
void noise_clock_frequency(PSG*);

// Divisor for the master clock frequency (actually, the frequency of the Z80)
const uint8_t MASTER_CYCLES_PER_PSG_CLOCK = 16;
const uint32_t SAMPLE_RATE = 44100;
const uint32_t NTSC_FREQUENCY = 3579545;

// FIXME: this is based on NTSC frequency
// (double)NTSC_FREQUENCY / (double)MASTER_CYCLES_PER_PSG_CLOCK / (double)SAMPLE_RATE;
// C doesn't have constexpr, so we have to hardcode this one:
const double PSG_CLOCKS_PER_SAMPLE = 5.07305130385;

const int16_t volume_table[16]= {
    32767, 26028, 20675, 16422, 13045, 10362,  8231,  6568,
    5193,  4125,  3277,  2603,  2067,  1642,  1304,     0
};

PSG* psg_make(struct Genesis* g) {
    return calloc(1, sizeof(PSG));
}

void psg_free(PSG* p) {
    free(p);
}

void psg_initialize(PSG* p) {
    // Reset
    memset(p, 0, sizeof(PSG));

    // Mute all channels
    p->square[0].volume = 0xf;
    p->square[1].volume = 0xf;
    p->square[2].volume = 0xf;
    p->noise.volume = 0xf;
}

// Entry point for other chip to communicate with the PSG
void psg_write(PSG* p, uint8_t value) {
    if (value & 0x80) {
        p->latched_channel = value >> 5;
        p->latched_register = value >> 4;
        write_latch(p, value);
    } else {
        write_data(p, value);
    }
}

// Called at NTSC_FREQUENCY
void psg_clock(PSG* p) {
    for (int i=0; i < 3; ++i) {
        square_clock_frequency(&p->square[i]);
    }

    noise_clock_frequency(p);

    // Is it time to emit a sample?
    if (p->sample_counter > 0) {
        p->sample_counter--;
    } else {
        p->sample_counter += PSG_CLOCKS_PER_SAMPLE;
        psg_emit_sample_cb(psg_mix(p));
    }
}

// Entry point used by the rest of emulator
// Cycles: number of master cycles to emulate
void psg_run_cycles(PSG* p, uint16_t cycles) {
    p->remaining_clocks += cycles;

    while (p->remaining_clocks > 0) {
        psg_clock(p);
        p->remaining_clocks -= MASTER_CYCLES_PER_PSG_CLOCK;
    }
}

// Called at PSG frequency
void square_clock_frequency(SquareChannel* s) {
    if (s->counter > 0) {
        s->counter--;
    }
    // Doc seems to state that reloading can happen in the same clock the
    // counter has gone to zero
    if (s->counter == 0) {
        // Reload counter
        s->counter = s->tone;

        // If the tone register is 0, output is always +1
        // XXX: the doc is ambiguous, as it states that a value of 1 also
        // outputs +1, but it also states that 1 corresponds to the highest
        // frequency that can be output.
        if (s->tone == 0) {
            s->output = 1;
        } else {
        // Otherwise flip output
            s->output = !s->output;
        }
    }
}

float square_tone_in_hertz(SquareChannel* s) {
    return (float)NTSC_FREQUENCY / (2.0f * (float)s->tone * (float)MASTER_CYCLES_PER_PSG_CLOCK);
}

// Returns the volume-attenuated output for this channel
int16_t square_output(SquareChannel* s) {
    return s->output ? volume_table[s->volume] : 0;
}

void noise_clock_frequency(PSG* p) {
    NoiseChannel* n = &p->noise;

    if (n->counter > 0) {
        n->counter--;
    }

    if (n->counter == 0) {
        // Reload counter
        switch (n->shift_rate) {
        case 0x00: n->counter = 0x10; break;
        case 0x01: n->counter = 0x20; break;
        case 0x02: n->counter = 0x40; break;
        case 0x03: n->counter = p->square[2].tone; break;
        }

        // Shift bit in LFSR
        uint8_t out = n->lfsr & 1;
        // Output bit is written back to input in periodic mode
        uint8_t in = out;
        if (n->mode == 1) {
            // In white noise mode, input is XORed with bit 3
            in ^= ((n->lfsr >> 3) & 1);
        }
        n->lfsr = (in << 15) | ((n->lfsr >> 1) & 0x7fff);

        n->output = out;
    }
}

int16_t noise_output(NoiseChannel* n) {
    return n->output ? volume_table[n->volume] : 0;
}

// Mix outputs from each channel
int16_t psg_mix(PSG* p) {
    int32_t sample = 0;

    // XXX: I have no idea if this is proper, but it seemed to work well enough
    // for Boyo
    sample += square_output(&p->square[0]);
    sample += square_output(&p->square[1]);
    sample += square_output(&p->square[2]);
    sample += noise_output(&p->noise);
    sample /= 4;

    return sample;
}

// Write low nibble to the currently latched channel & register
void write_latch(PSG* p, uint8_t value) {
    value = value & 0xf;
    switch (p->latched_channel) {
    case 0:
    case 1:
    case 2:
        if (p->latched_register) {
            p->square[p->latched_channel].volume = value;
        } else {
            p->square[p->latched_channel].tone = (p->square[p->latched_channel].tone & 0x3f0) | value;
        }
        break;

    case 3:
        if (p->latched_register) {
            p->noise.volume = value;
        } else {
            p->noise.noise = value;
            // Writing to the noise register resets the LFSR
            p->noise.lfsr = 0x8000;
        }
        break;
    }
}

// Write 6bit value to the currently latched channel & register
void write_data(PSG* p, uint8_t value) {
    value = value & 0x3f;
    switch (p->latched_channel) {
    case 0:
    case 1:
    case 2:
        if (p->latched_register) {
            p->square[p->latched_channel].volume = value;
        } else {
            p->square[p->latched_channel].tone = (p->square[p->latched_channel].tone & 0xf) | (value << 4);
        }
        break;

    case 3:
        if (p->latched_register) {
            p->noise.volume = value;
        } else {
            p->noise.noise = value;
            // Writing to the noise register resets the LFSR
            p->noise.lfsr = 0x8000;
        }
        break;
    }
}

// Buffer of 100 seconds
#define MAX_SAMPLES 4410000
int16_t samples[MAX_SAMPLES];
uint32_t samples_cursor = 0;

// TEMP: move to a proper audio backend
void psg_emit_sample_cb(uint16_t sample) {
    // Fill the buffer then stop
    if (samples_cursor < MAX_SAMPLES)
        samples[samples_cursor++] = sample;
    //samples_cursor = (samples_cursor + 1) % MAX_SAMPLES;
}

// TEMP: Write samples to a WAV file
// src: https://codereview.stackexchange.com/q/105272

#include <stdbool.h>

const char fChunkID[]     = {'R', 'I', 'F', 'F'};
const char fFormat[]      = {'W', 'A', 'V', 'E'};
const char fSubchunk1ID[] = {'f', 'm', 't', ' '};
const char fSubchunk2ID[] = {'d', 'a', 't', 'a'};

const unsigned short N_CHANNELS = 1;
const unsigned short BITS_PER_BYTE = 8;

bool wav_write(const char* fileName){
    static const unsigned int fSubchunk1Size = 16;
    static const unsigned short fAudioFormat = 1;
    static const unsigned short fBitsPerSample = 16;

    unsigned int fByteRate = SAMPLE_RATE * N_CHANNELS *
                             fBitsPerSample / BITS_PER_BYTE;

    unsigned short fBlockAlign = N_CHANNELS * fBitsPerSample / BITS_PER_BYTE;
    unsigned int fSubchunk2Size;
    unsigned int fChunkSize;

    FILE* fout;

    if (!fileName || !(fout = fopen( fileName, "w" ))) return false;

    fSubchunk2Size = MAX_SAMPLES * N_CHANNELS * fBitsPerSample / BITS_PER_BYTE;
    fChunkSize = 36 + fSubchunk2Size;

    // Writing the RIFF header:
    fwrite(&fChunkID, 1, sizeof(fChunkID),      fout);
    fwrite(&fChunkSize,  sizeof(fChunkSize), 1, fout);
    fwrite(&fFormat, 1,  sizeof(fFormat),       fout);

    // "fmt" chunk:
    fwrite(&fSubchunk1ID, 1, sizeof(fSubchunk1ID),      fout);
    fwrite(&fSubchunk1Size,  sizeof(fSubchunk1Size), 1, fout);
    fwrite(&fAudioFormat,    sizeof(fAudioFormat),   1, fout);
    fwrite(&N_CHANNELS,      sizeof(N_CHANNELS),     1, fout);
    fwrite(&SAMPLE_RATE,     sizeof(SAMPLE_RATE),    1, fout);
    fwrite(&fByteRate,       sizeof(fByteRate),      1, fout);
    fwrite(&fBlockAlign,     sizeof(fBlockAlign),    1, fout);
    fwrite(&fBitsPerSample,  sizeof(fBitsPerSample), 1, fout);

    /* "data" chunk: */
    fwrite(&fSubchunk2ID, 1, sizeof(fSubchunk2ID),      fout);
    fwrite(&fSubchunk2Size,  sizeof(fSubchunk2Size), 1, fout);

    /* sound data: */
    fwrite(samples, sizeof(int16_t), MAX_SAMPLES * N_CHANNELS, fout);
    fclose(fout);
    return true;
}
