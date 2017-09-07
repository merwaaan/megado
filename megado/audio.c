#include <SDL2/SDL.h>

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
    want.channels = 1;
    want.samples = 512;
    want.callback = NULL; // Use SDL_QueueAudio instead

    a->device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);

    if (a->device == 0) {
        fprintf(stderr, "Failed to open audio: %s", SDL_GetError());
        exit(1);
    }

    return a;
}

void audio_free(Audio* a) {
    SDL_CloseAudioDevice(a->device);
    SDL_Quit();

    free(a);
}

void audio_initialize(Audio* a) {
    SDL_ClearQueuedAudio(a->device);
}

void audio_update(Audio* a) {
    if (a->genesis->status == Status_Running) {
        SDL_PauseAudioDevice(a->device, 0);
    } else {
        SDL_PauseAudioDevice(a->device, 1);
    }
}
