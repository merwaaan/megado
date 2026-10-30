#include <print>
#include <SDL.h>

#include "audio.h"

using namespace std;

Audio::Audio()
{
    SDL_Init(SDL_INIT_AUDIO);

    SDL_AudioSpec want{};
    want.freq = SAMPLE_RATE;
    want.format = AUDIO_S16;
    want.channels = 2;
    want.samples = 512;
    want.callback = nullptr; // We use SDL_QueueAudio to push samples instead

    SDL_AudioSpec have;
    device = SDL_OpenAudioDevice(nullptr, 0, &want, &have, 0);

    if (device == 0)
    {
        println(stderr, "Failed to open audio: {}", SDL_GetError());
        println(stderr, "Audio disabled");
    }
}

Audio::~Audio()
{
    if (device > 0)
    {
        SDL_CloseAudioDevice(device);
    }

    SDL_Quit();
}

void Audio::reset()
{
    if (device > 0)
    {
        SDL_ClearQueuedAudio(device);
    }
}

void Audio::set_paused(bool paused)
{
    if (device > 0)
    {
        SDL_PauseAudioDevice(device, paused ? 1 : 0);
    }
}

void Audio::queue_samples(span<const int16_t> samples)
{
    if (device > 0 && SDL_QueueAudio(device, samples.data(), samples.size_bytes()) != 0)
    {
        print(stderr, "Failed to queue audio: {}", SDL_GetError());
    }
}

uint32_t Audio::queued_samples() const
{
    return SDL_GetQueuedAudioSize(device) / sizeof(int16_t);
}
