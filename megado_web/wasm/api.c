#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "megado_core/genesis.h"
#include "megado_core/snapshot.h"
#include "megado_core/vdp.h"

static Genesis* genesis;
static int16_t* samples;
static uint32_t samples_capacity;
static char rom_name[49];

void megado_create(void)
{
    genesis = genesis_make();
}

void megado_destroy(void)
{
    genesis_free(genesis);
    genesis = NULL;

    free(samples);
    samples = NULL;
    samples_capacity = 0;
}

void megado_load_rom(const uint8_t* data, uint32_t size)
{
    genesis_load_rom(genesis, data, size);
}

const char* megado_rom_name(void)
{
    genesis_get_rom_name(genesis, rom_name);
    return rom_name;
}

uint32_t megado_status(void)
{
    return genesis->status;
}

uint32_t megado_run_samples(uint32_t count, double sample_duration)
{
    if (count > samples_capacity)
    {
        samples = realloc(samples, count * 2 * sizeof(int16_t));
        samples_capacity = count;
    }

    return genesis_run_samples(genesis, samples, count, sample_duration);
}

int16_t* megado_samples(void)
{
    return samples;
}

uint8_t* megado_frame(void)
{
    uint16_t width, height;
    return genesis_framebuffer(genesis, &width, &height);
}

uint32_t megado_frame_width(void)
{
    uint16_t width, height;
    genesis_framebuffer(genesis, &width, &height);
    return width;
}

uint32_t megado_frame_height(void)
{
    uint16_t width, height;
    genesis_framebuffer(genesis, &width, &height);
    return height;
}

uint32_t megado_frame_pitch(void)
{
    return BUFFER_WIDTH * 3;
}

void megado_set_button(uint32_t port, uint32_t button, bool pressed)
{
    genesis_set_button(genesis, port, button, pressed);
}

uint32_t megado_snapshot_version(void)
{
    return SNAPSHOT_VERSION;
}

uint32_t megado_snapshot_size(void)
{
    return sizeof(Snapshot);
}

uint8_t* megado_snapshot_take(void)
{
    return (uint8_t*)snapshot_take(genesis);
}

void megado_snapshot_restore(const uint8_t* data)
{
    Snapshot* snapshot = malloc(sizeof(Snapshot));
    memcpy(snapshot, data, sizeof(Snapshot));
    snapshot_restore(genesis, snapshot);
    free(snapshot);
}
