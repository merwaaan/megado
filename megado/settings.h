#pragma once

#include <stdbool.h>
#include <stdint.h>

#define SETTINGS_FILE "settings.json"

// Bump that version number when changing the format
// in order to discard out of date settings
#define SETTINGS_FORMAT_VERSION 0

typedef struct Settings
{
    // Video settings
    float video_scale; // Scaling factor for the Genesis video output
    bool vsync;

    // UI settings
    bool show_cpu_registers;
    bool show_cpu_disassembly;
    bool show_vdp_registers;
    bool show_vdp_palettes;
    bool show_vdp_patterns;
    bool show_vdp_planes;
    bool show_vdp_sprites;
    bool show_rom;
    bool show_ram;
    bool show_vram;
    bool show_vsram;
    bool show_cram;

    // Breakpoints
    uint32_t breakpoints[3];
    // TODO keep checkpoints for all games (key = name in header? checksum?)
} Settings;

Settings* settings_make();
void settings_free(Settings*);

void settings_save(Settings*);
Settings* settings_load();
