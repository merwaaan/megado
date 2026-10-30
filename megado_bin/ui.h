#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "core.h"
#include "metric.h"
#include "snapshot.h"

class Audio;
class Settings;

struct UiEvents
{
    bool reset = false;
    bool toggle_pause = false;
    bool step = false;
    bool toggle_full_screen = false;
    std::optional<uint8_t> save_snapshot;
    std::optional<uint8_t> load_snapshot;
};

class Ui
{
public:
    Ui();
    ~Ui();

    Ui(const Ui&) = delete;
    Ui& operator=(const Ui&) = delete;

    UiEvents draw(Genesis* genesis, Settings& settings, const Audio& audio, const SnapshotSlots& snapshot_slots, double remaining_time);

private:
    float last_time = 0;

    // Metrics
    float metrics_refresh_counter = 0;
    Metric time_per_frame{128};
    Metric audio_buffer_queue{128};

    // Inspected memory addresses
    uint32_t rom_target_address = 0;
    uint32_t ram_target_address = 0;
    uint32_t vram_target_address = 0;

    // Graphics debug
    int patterns_palette_index = -1;
    enum Planes selected_plane = Plane_A;
    std::vector<uint8_t> plane_buffer;
    std::vector<uint8_t> sprites_buffer;

    // Textures for debug UI views
    unsigned int patterns_texture = 0;
    unsigned int magnified_pattern_texture = 0;
    unsigned int planes_texture = 0;
    unsigned int sprites_texture = 0;
};
