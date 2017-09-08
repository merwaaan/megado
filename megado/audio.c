#include <SDL.h>
#include <stdio.h>

#include "audio.h"
#include "genesis.h"

Audio* audio_make(Genesis* g) {
    Audio* a = calloc(1, sizeof(Audio));
    a->genesis = g;

    SDL_Init(SDL_INIT_AUDIO);

    SDL_AudioSpec want, have;

    SDL_memset(&want, 0, sizeof(want));
    want.freq = 44100;
    want.format = AUDIO_S16;
    want.channels = 2;
    want.samples = 512;
    want.callback = NULL; // Use SDL_QueueAudio instead

    a->device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);

    if (a->device == 0) {
        fprintf(stderr, "Failed to open audio: %s\n", SDL_GetError());
        fprintf(stderr, "Continuing with audio disabled\n");
    }

    return a;
}

void audio_free(Audio* a) {
    if (a->device > 0) {
        SDL_CloseAudioDevice(a->device);
    }
    SDL_Quit();

    free(a);
}

void audio_initialize(Audio* a) {
    if (a->device > 0) {
        SDL_ClearQueuedAudio(a->device);
    }

    memset(a->ym2612_samples, 0, sizeof(a->ym2612_samples));
    a->ym2612_sample_write_cursor = 0;
    a->ym2612_sample_read_cursor = 0;
    memset(a->psg_samples, 0, sizeof(a->psg_samples));
    a->psg_sample_write_cursor = 0;
    a->psg_sample_read_cursor = 0;
}

void audio_update(Audio* a) {
    if (a->genesis->status == Status_Running) {
        if (a->device > 0) {
            SDL_PauseAudioDevice(a->device, 0);
        }
    } else {
        if (a->device > 0) {
            SDL_PauseAudioDevice(a->device, 1);
        }
    }

    // Output PSG to the left, YM2612 to the right
    int16_t mixed_buffer[1024];

    int i = 0;
    while (a->psg_sample_read_cursor != a->psg_sample_write_cursor
           && a->ym2612_sample_read_cursor != a->ym2612_sample_write_cursor
           && i < 1024) {
        mixed_buffer[i++] = a->psg_samples   [a->psg_sample_read_cursor];
        mixed_buffer[i++] = a->ym2612_samples[a->ym2612_sample_read_cursor];
        a->psg_sample_read_cursor    = (a->psg_sample_read_cursor    + 1) % 512;
        a->ym2612_sample_read_cursor = (a->ym2612_sample_read_cursor + 1) % 512;
    }

    // Queue buffer
    if (i > 0 && a->device > 0 && SDL_QueueAudio(a->device, mixed_buffer, i * sizeof(int16_t)) != 0) {
        fprintf(stderr, "Failed to queue audio: %s", SDL_GetError());
    }
}
