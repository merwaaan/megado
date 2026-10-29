#pragma once

#include <stdbool.h>
#include <stdint.h>

extern const uint32_t SAMPLE_RATE;

typedef struct Audio
{
    uint32_t device;
    double remaining_time;
} Audio;

Audio* audio_make();
void audio_free(Audio*);
void audio_initialize(Audio*);
void audio_set_paused(Audio*, bool paused);
void audio_queue(Audio*, int16_t* samples, uint32_t count);
uint32_t audio_queued_samples(Audio*);
