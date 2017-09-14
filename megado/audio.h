#pragma once

#include <SDL.h>

extern const uint32_t SAMPLE_RATE;

struct Genesis;

typedef struct Audio {
    struct Genesis* genesis;
    SDL_AudioDeviceID device;
    double remaining_time;
} Audio;

Audio* audio_make(struct Genesis*);
void audio_free(Audio*);
void audio_initialize(Audio*);
void audio_update(Audio*);
