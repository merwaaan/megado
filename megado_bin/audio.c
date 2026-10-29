#include <SDL.h>
#include <stdio.h>

#include "audio.h"

const uint32_t SAMPLE_RATE = 44100;

Audio* audio_make()
{
    Audio* a = calloc(1, sizeof(Audio));

    SDL_Init(SDL_INIT_AUDIO);

    SDL_AudioSpec want, have;

    SDL_memset(&want, 0, sizeof(want));
    want.freq = SAMPLE_RATE;
    want.format = AUDIO_S16;
    want.channels = 2;
    want.samples = 512;
    want.callback = NULL; // Use SDL_QueueAudio instead

    a->device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);

    if (a->device == 0)
    {
        fprintf(stderr, "Failed to open audio: %s\n", SDL_GetError());
        fprintf(stderr, "Audio disabled\n");
    }

    return a;
}

void audio_free(Audio* a)
{
    if (a->device > 0)
    {
        SDL_CloseAudioDevice(a->device);
    }

    SDL_Quit();

    free(a);
}

void audio_initialize(Audio* a)
{
    if (a->device > 0)
    {
        SDL_ClearQueuedAudio(a->device);
    }

    a->remaining_time = 0;
}

void audio_set_paused(Audio* a, bool paused)
{
    if (a->device > 0)
    {
        SDL_PauseAudioDevice(a->device, paused ? 1 : 0);
    }
}

void audio_queue(Audio* a, int16_t* samples, uint32_t count)
{
    if (a->device > 0 && SDL_QueueAudio(a->device, samples, count * 2 * sizeof(int16_t)) != 0)
    {
        fprintf(stderr, "Failed to queue audio: %s", SDL_GetError());
    }
}

uint32_t audio_queued_samples(Audio* a)
{
    return SDL_GetQueuedAudioSize(a->device) / sizeof(int16_t);
}
