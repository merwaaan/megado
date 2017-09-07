#pragma once

#include <SDL2/SDL.h>

struct Genesis;

typedef struct Audio {
    SDL_AudioDeviceID device;
} Audio;

Audio* audio_make(struct Genesis*);
void audio_free(Audio*);
