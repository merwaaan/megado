#pragma once

#include <filesystem>

#include "audio.h"
#include "core.h"
#include "settings.h"
#include "ui.h"
#include "video.h"

class App
{
public:
    App();
    ~App();

    App(const App&) = delete;
    App& operator=(const App&) = delete;

    void load_rom_file(const std::filesystem::path& path);

    void run();

private:
    Genesis* genesis = genesis_make();
    Settings settings = Settings::load();
    Audio audio;
    Video video;
    Ui ui;
    SnapshotSlots snapshot_slots;

    double remaining_time = 0;

    VideoCallbacks video_callbacks();
};
