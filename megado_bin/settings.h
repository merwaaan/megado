#pragma once

#include <array>
#include <map>
#include <nlohmann/json.hpp>
#include <string>

#include "core.h"

#define SETTINGS_FILE "settings.json"

// Bump that version number when changing the format in order to discard out of date settings
#define SETTINGS_FORMAT_VERSION 3

using Breakpoints = std::array<Breakpoint, BREAKPOINTS_COUNT>;

class Settings
{
public:
    static Settings load();
    void save() const;

    // Returns breakpoints for the given game.
    Breakpoint* breakpoints_for(const std::string& game); // TODO use hash?

    int window_width = 1200;
    int window_height = 800;

    bool show_m68k_registers = false;
    bool show_m68k_disassembly = false;
    bool show_m68k_log = false;
    bool show_z80_disassembly = false;
    bool show_z80_log = false;
    bool show_z80_registers = false;
    bool show_vdp_registers = false;
    bool show_vdp_palettes = false;
    bool show_vdp_patterns = false;
    bool show_vdp_planes = false;
    bool show_vdp_sprites = false;
    bool show_psg_registers = false;
    bool show_ym2612_registers = false;
    bool show_rom = false;
    bool show_ram = false;
    bool show_vram = false;
    bool show_vsram = false;
    bool show_cram = false;
    bool show_metrics = false;
    float video_scale = 1.0f;
    bool full_screen = false;
    bool rewinding_enabled = false;
    float emulation_speed = 1.0f;

    std::map<std::string, Breakpoints> breakpoints;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(
        Settings,
        window_width,
        window_height,
        video_scale,
        emulation_speed,
        show_m68k_registers,
        show_m68k_disassembly,
        show_m68k_log,
        show_z80_disassembly,
        show_z80_log,
        show_z80_registers,
        show_vdp_registers,
        show_vdp_palettes,
        show_vdp_patterns,
        show_vdp_planes,
        show_vdp_sprites,
        show_psg_registers,
        show_ym2612_registers,
        show_rom,
        show_ram,
        show_vram,
        show_vsram,
        show_cram,
        show_metrics,
        rewinding_enabled)
};
