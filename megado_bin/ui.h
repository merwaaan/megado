#pragma once

#include <stdint.h>

#include "snapshot.h"
#include "vdp.h"

struct Audio;
struct Genesis;
struct Settings;

typedef enum
{
    UiAction_None = 0,
    UiAction_TogglePause = 1 << 0,
    UiAction_Step = 1 << 1,
    UiAction_ToggleFullScreen = 1 << 2
} UiAction;

typedef struct Ui
{
    struct SnapshotMetadata* snapshots[SNAPSHOT_SLOTS];

    float last_time;
    float metrics_refresh_counter;
    struct Metric* tpf; // time per frame
    struct Metric* audio_buffer_queue;

    enum Planes selected_plane;
    uint8_t* plane_buffer;
    uint8_t* sprites_buffer;

    uint32_t rom_target_address;
    uint32_t ram_target_address;
    uint32_t vram_target_address;

    // Textures for debug UI views (ImGui draws via imgui_impl)
    unsigned int patterns_texture, magnified_pattern_texture, planes_texture, sprites_texture;
} Ui;

Ui* ui_make();
void ui_free(Ui*);

void ui_load_snapshots(Ui*, struct Genesis*);
int ui_draw(Ui*, struct Genesis*, struct Settings*, struct Audio*);
