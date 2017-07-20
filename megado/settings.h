#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "debugger.h"

#define SETTINGS_FILE "settings.json"

// Bump that version number when changing the format
// in order to discard out of date settings
#define SETTINGS_FORMAT_VERSION 3

typedef struct BreakpointSet
{
    char game[49];
    Breakpoint breakpoints[BREAKPOINTS_COUNT];
} BreakpointSet;

typedef struct Settings
{
    int window_width;
    int window_height;

    // Video settings
    float video_scale; // Scaling factor for the Genesis video output
    bool full_screen;
    bool vsync;

    // UI settings
    bool show_m68k_registers;
    bool show_m68k_disassembly;
    bool show_m68k_log;
    bool show_z80_disassembly;
    bool show_z80_log;
    bool show_z80_registers;
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
    bool show_metrics;

    bool rewinding_enabled;

    BreakpointSet* breakpoint_sets;
    int breakpoint_sets_length;
} Settings;

Settings* settings_make();
void settings_free(Settings*);

void settings_save(Settings*);
Settings* settings_load();

// Returns breakpoints for the given game.
Breakpoint* settings_get_or_create_breakpoints(Settings*, char* game);
