#include <cstdlib>
#include <fstream>
#include <GLFW/glfw3.h>
#include <iterator>
#include <print>
#include <vector>

#include "app.h"

using namespace std;

#define REWIND_PLAY_INTERVAL 0.05

static void toggle_pause(Genesis* g)
{
    if (g->status == Status_Running)
    {
        g->status = Status_Pause;
    }
    else if (g->status == Status_Pause)
    {
        g->status = Status_Running;
    }
}

static void step(Genesis* g)
{
    if (g->status == Status_Pause)
    {
        genesis_step(g);
    }
}

static void toggle_full_screen(Settings& settings, Video& video)
{
    settings.full_screen = !settings.full_screen;

    video.set_full_screen(settings.full_screen);
}

static void set_button(Genesis* g, int key, bool pressed)
{
    switch (key)
    {
    case GLFW_KEY_LEFT:
        genesis_set_button(g, 0, Left, pressed);
        break;
    case GLFW_KEY_RIGHT:
        genesis_set_button(g, 0, Right, pressed);
        break;
    case GLFW_KEY_UP:
        genesis_set_button(g, 0, Up, pressed);
        break;
    case GLFW_KEY_DOWN:
        genesis_set_button(g, 0, Down, pressed);
        break;
    case GLFW_KEY_ENTER:
        genesis_set_button(g, 0, Start, pressed);
        break;
    case GLFW_KEY_Q:
        genesis_set_button(g, 0, ButtonA, pressed);
        break;
    case GLFW_KEY_W:
        genesis_set_button(g, 0, ButtonB, pressed);
        break;
    case GLFW_KEY_E:
        genesis_set_button(g, 0, ButtonC, pressed);
        break;

    case GLFW_KEY_1:
        genesis_set_button(g, 1, Left, pressed);
        break;
    case GLFW_KEY_3:
        genesis_set_button(g, 1, Right, pressed);
        break;
    case GLFW_KEY_5:
        genesis_set_button(g, 1, Up, pressed);
        break;
    case GLFW_KEY_2:
        genesis_set_button(g, 1, Down, pressed);
        break;
    case GLFW_KEY_0:
        genesis_set_button(g, 1, Start, pressed);
        break;
    case GLFW_KEY_7:
        genesis_set_button(g, 1, ButtonA, pressed);
        break;
    case GLFW_KEY_8:
        genesis_set_button(g, 1, ButtonB, pressed);
        break;
    case GLFW_KEY_9:
        genesis_set_button(g, 1, ButtonC, pressed);
        break;
    }
}

static void on_key(Genesis* g, Settings& settings, Video& video, int key, int action)
{
    switch (action)
    {
    case GLFW_PRESS:
        set_button(g, key, true);

        if (key == GLFW_KEY_R && settings.rewinding_enabled && g->status == Status_Running)
        {
            g->status = Status_Rewinding;
        }
        break;
    case GLFW_RELEASE:
        set_button(g, key, false);

        switch (key)
        {
        case GLFW_KEY_F:
            toggle_full_screen(settings, video);
            break;
        case GLFW_KEY_P:
            toggle_pause(g);
            break;
        case GLFW_KEY_SPACE:
            step(g);
            break;

        case GLFW_KEY_ESCAPE:
            if (settings.full_screen)
            {
                toggle_full_screen(settings, video);
            }
            else
            {
                g->status = Status_Quitting;
            }
            break;

        case GLFW_KEY_R:
            if (g->status == Status_Rewinding)
            {
                g->status = Status_Running;
            }
            break;
        }
        break;
    }
}

App::App()
    : video(settings.window_width, settings.window_height, video_callbacks())
{
}

App::~App()
{
    // Save the settings on exit
    settings.save();

    genesis_free(genesis);
}

void App::load_rom_file(const filesystem::path& path)
{
    println("Opening {}...", path.string());

    ifstream file(path, ios::binary);

    if (!file)
    {
        println(stderr, "Cannot read file \"{}\"", path.string());
        exit(1);
    }

    // Load the ROM into memory

    vector<uint8_t> data(istreambuf_iterator<char>(file), {});

    if (!genesis_load_rom(genesis, data.data(), data.size()))
    {
        println(stderr, "Cannot load ROM from \"{}\"", path.string());
        return;
    }

    // Reset audio

    audio.reset();
    remaining_time = 0;

    // Load snapshots/breakpoints for this game

    snapshot_slots = snapshots::preload(genesis);

    genesis->debugger->breakpoints = settings.breakpoints_for(rom_name(genesis));
}

void App::run()
{
    double last_update = 0;
    double last_rewind = 0;

    while (genesis->status != Status_Quitting)
    {
        // dt is wall time in seconds elapsed since last update
        double now = glfwGetTime();
        double dt = now - last_update;
        last_update = now;

        // Schedule the end of this frame from the previous frame time
        // (with some slack for overhead)
        double max_time = now + dt - (dt / 10);

        genesis->debugger->m68k_log_enabled = settings.show_m68k_log;
        genesis->debugger->z80_log_enabled = settings.show_z80_log;
        genesis->debugger->rewinding_enabled = settings.rewinding_enabled;

        if (genesis->status == Status_Running)
        {
            // Emulate by slices of audio sample
            remaining_time += dt;
            double time_slice = 1.0 / Audio::SAMPLE_RATE;

            // How many Genesis seconds we need to emulate, depending on speed factor
            double dt_genesis = time_slice * settings.emulation_speed;

            while (remaining_time > 0)
            {
                // Emulate enough for one audio sample
                int16_t sample[2];
                genesis_run_samples(genesis, sample, 1, dt_genesis);

                // Queue samples to audio device
                audio.queue_samples(sample);

                remaining_time -= time_slice;

                // Exit early on breakpoint
                if (genesis->status != Status_Running)
                {
                    break;
                }

                // If we are taking longer than the allocated time, abort
                if (glfwGetTime() > max_time)
                {
                    break;
                }
            }
        }
        else if (genesis->status == Status_Rewinding)
        {
            if (now - last_rewind >= REWIND_PLAY_INTERVAL)
            {
                bool more_to_rewind = debugger_rewind(genesis->debugger);
                if (!more_to_rewind)
                {
                    genesis->status = Status_Running;
                }

                last_rewind = now;
            }

            vdp_draw_screen(genesis->vdp);
        }

        // Draw

        video.begin_frame();

        UiEvents events = ui.draw(genesis, settings, audio, snapshot_slots, remaining_time);

        uint16_t width, height;
        uint8_t* pixels = genesis_framebuffer(genesis, &width, &height);
        video.draw({pixels, static_cast<size_t>(BUFFER_WIDTH * 3 * height)}, width, height, BUFFER_WIDTH * 3, settings.video_scale);

        video.end_frame();

        // Handle UI events

        if (events.reset)
        {
            genesis_initialize(genesis);
            audio.reset();
            remaining_time = 0;
        }

        if (events.toggle_pause)
        {
            toggle_pause(genesis);
        }

        if (events.step)
        {
            step(genesis);
        }

        if (events.toggle_full_screen)
        {
            toggle_full_screen(settings, video);
        }

        if (events.save_snapshot)
        {
            snapshot_slots[*events.save_snapshot] = snapshots::save(genesis, *events.save_snapshot);
        }

        if (events.load_snapshot)
        {
            snapshots::load(genesis, *events.load_snapshot);
        }

        // Toggle audio depending on the emulation status

        audio.set_paused(genesis->status != Status_Running);
    }
}

VideoCallbacks App::video_callbacks()
{
    VideoCallbacks callbacks;

    callbacks.key = [this](int key, int action)
    {
        on_key(genesis, settings, video, key, action);
    };

    callbacks.drop = [this](const char* path)
    {
        load_rom_file(path);
    };

    callbacks.resize = [this](int width, int height)
    {
        settings.window_width = width;
        settings.window_height = height;
    };

    callbacks.close = [this]()
    {
        genesis->status = Status_Quitting;
    };

    return callbacks;
}
