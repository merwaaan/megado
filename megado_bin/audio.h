#pragma once

#include <cstdint>
#include <span>

class Audio
{
public:
    static constexpr uint32_t SAMPLE_RATE = 44100;

    Audio();
    ~Audio();

    Audio(const Audio&) = delete;
    Audio& operator=(const Audio&) = delete;

    void reset();
    void set_paused(bool paused);
    void queue_samples(std::span<const int16_t> samples);
    uint32_t queued_samples() const;

private:
    uint32_t device = 0;
};
