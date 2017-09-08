#pragma once

#include <SDL.h>

struct Genesis;

typedef struct Audio {
    struct Genesis* genesis;
    SDL_AudioDeviceID device;

    int16_t ym2612_samples[512];
    uint32_t ym2612_sample_write_cursor;
    uint32_t ym2612_sample_read_cursor;
    int16_t psg_samples[512];
    uint32_t psg_sample_write_cursor;
    uint32_t psg_sample_read_cursor;
} Audio;

Audio* audio_make(struct Genesis*);
void audio_free(Audio*);
void audio_initialize(Audio*);
void audio_update(Audio*);
