#pragma once

#include <SDL2/SDL.h>

struct Genesis;

typedef struct Audio {
    struct Genesis* genesis;
    SDL_AudioDeviceID device;
} Audio;

Audio* audio_make(struct Genesis*);
void audio_free(Audio*);
void audio_initialize(Audio*);
void audio_update(Audio*);
