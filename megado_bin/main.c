#include <GLFW/glfw3.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "audio.h"
#include "megado_core/debugger.h"
#include "megado_core/genesis.h"
#include "megado_core/joypad.h"
#include "megado_core/utils.h"
#include "megado_core/vdp.h"
#include "settings.h"
#include "ui.h"
#include "video.h"

#define REWIND_PLAY_INTERVAL 0.05

typedef struct App
{
    Genesis* genesis;
    Settings* settings;
    Audio* audio;
    Video* video;
    Ui* ui;
} App;

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

static void toggle_full_screen(App* app)
{
    app->settings->full_screen = !app->settings->full_screen;
    video_set_full_screen(app->video, app->settings->full_screen);
}

static void apply_actions(App* app, int actions)
{
    if (actions & UiAction_TogglePause)
    {
        toggle_pause(app->genesis);
    }

    if (actions & UiAction_Step)
    {
        step(app->genesis);
    }

    if (actions & UiAction_ToggleFullScreen)
    {
        toggle_full_screen(app);
    }
}

static void load_rom_file(App* app, const char* path)
{
    printf("Opening %s...\n", path);

    FILE* file = fopen(path, "rb");

    if (!file)
    {
        FATAL("Cannot read file \"%s\"", path);
    }

    // Load the ROM into memory

    fseek(file, 0, SEEK_END);
    long file_length = ftell(file);

    fseek(file, 0, SEEK_SET);
    uint8_t* data = malloc(file_length);
    fread(data, sizeof(uint8_t), file_length, file);

    fclose(file);

    genesis_load_rom(app->genesis, data, file_length);

    free(data);

    // Reset audio

    audio_initialize(app->audio);

    // Load snapshots/breakpoints for this game

    ui_load_snapshots(app->ui, app->genesis);

    char name[49];
    genesis_get_rom_name(app->genesis, name);
    app->genesis->debugger->breakpoints = settings_get_or_create_breakpoints(app->settings, name);
}

// TODO move this to genesis (via callback)
static void on_key(void* app_ptr, int key, int action)
{
    App* app = app_ptr;

    switch (action)
    {
    case GLFW_PRESS:
        switch (key)
        {
        case GLFW_KEY_LEFT:
            joypad_press(app->genesis->joypad1, Left);
            break;
        case GLFW_KEY_RIGHT:
            joypad_press(app->genesis->joypad1, Right);
            break;
        case GLFW_KEY_UP:
            joypad_press(app->genesis->joypad1, Up);
            break;
        case GLFW_KEY_DOWN:
            joypad_press(app->genesis->joypad1, Down);
            break;
        case GLFW_KEY_ENTER:
            joypad_press(app->genesis->joypad1, Start);
            break;
        case GLFW_KEY_Q:
            joypad_press(app->genesis->joypad1, ButtonA);
            break;
        case GLFW_KEY_W:
            joypad_press(app->genesis->joypad1, ButtonB);
            break;
        case GLFW_KEY_E:
            joypad_press(app->genesis->joypad1, ButtonC);
            break;

        case GLFW_KEY_1:
            joypad_press(app->genesis->joypad2, Left);
            break;
        case GLFW_KEY_3:
            joypad_press(app->genesis->joypad2, Right);
            break;
        case GLFW_KEY_5:
            joypad_press(app->genesis->joypad2, Up);
            break;
        case GLFW_KEY_2:
            joypad_press(app->genesis->joypad2, Down);
            break;
        case GLFW_KEY_0:
            joypad_press(app->genesis->joypad2, Start);
            break;
        case GLFW_KEY_7:
            joypad_press(app->genesis->joypad2, ButtonA);
            break;
        case GLFW_KEY_8:
            joypad_press(app->genesis->joypad2, ButtonB);
            break;
        case GLFW_KEY_9:
            joypad_press(app->genesis->joypad2, ButtonC);
            break;

        case GLFW_KEY_R:
            if (app->settings->rewinding_enabled && app->genesis->status == Status_Running)
            {
                app->genesis->status = Status_Rewinding;
            }
            break;
        }
        break;
    case GLFW_RELEASE:
        switch (key)
        {
        case GLFW_KEY_LEFT:
            joypad_release(app->genesis->joypad1, Left);
            break;
        case GLFW_KEY_RIGHT:
            joypad_release(app->genesis->joypad1, Right);
            break;
        case GLFW_KEY_UP:
            joypad_release(app->genesis->joypad1, Up);
            break;
        case GLFW_KEY_DOWN:
            joypad_release(app->genesis->joypad1, Down);
            break;
        case GLFW_KEY_ENTER:
            joypad_release(app->genesis->joypad1, Start);
            break;
        case GLFW_KEY_Q:
            joypad_release(app->genesis->joypad1, ButtonA);
            break;
        case GLFW_KEY_W:
            joypad_release(app->genesis->joypad1, ButtonB);
            break;
        case GLFW_KEY_E:
            joypad_release(app->genesis->joypad1, ButtonC);
            break;

        case GLFW_KEY_1:
            joypad_release(app->genesis->joypad2, Left);
            break;
        case GLFW_KEY_3:
            joypad_release(app->genesis->joypad2, Right);
            break;
        case GLFW_KEY_5:
            joypad_release(app->genesis->joypad2, Up);
            break;
        case GLFW_KEY_2:
            joypad_release(app->genesis->joypad2, Down);
            break;
        case GLFW_KEY_0:
            joypad_release(app->genesis->joypad2, Start);
            break;
        case GLFW_KEY_7:
            joypad_release(app->genesis->joypad2, ButtonA);
            break;
        case GLFW_KEY_8:
            joypad_release(app->genesis->joypad2, ButtonB);
            break;
        case GLFW_KEY_9:
            joypad_release(app->genesis->joypad2, ButtonC);
            break;

        case GLFW_KEY_F:
            toggle_full_screen(app);
            break;
        case GLFW_KEY_P:
            toggle_pause(app->genesis);
            break;
        case GLFW_KEY_SPACE:
            step(app->genesis);
            break;

        case GLFW_KEY_ESCAPE:
            if (app->settings->full_screen)
            {
                toggle_full_screen(app);
            }
            else
            {
                app->genesis->status = Status_Quitting;
            }
            break;

        case GLFW_KEY_R:
            if (app->genesis->status == Status_Rewinding)
            {
                app->genesis->status = Status_Running;
            }
            break;
        }
        break;
    }
}

static void on_drop(void* app_ptr, const char* path)
{
    load_rom_file(app_ptr, path);
}

static void on_resize(void* app_ptr, int width, int height)
{
    App* app = app_ptr;
    app->settings->window_width = width;
    app->settings->window_height = height;
}

static void on_close(void* app_ptr)
{
    App* app = app_ptr;
    app->genesis->status = Status_Quitting;
}

static void update(App* app)
{
    Genesis* g = app->genesis;
    Audio* a = app->audio;
    Settings* s = app->settings;

    // dt is wall time in seconds elapsed since last update
    static double last_update = 0;
    static double last_rewind = 0;
    double now = glfwGetTime();
    double dt = now - last_update;
    last_update = now;

    // Schedule the end of this frame from the previous frame time
    // (with some slack for overhead)
    double max_time = now + dt - (dt / 10);

    g->debugger->m68k_log_enabled = s->show_m68k_log;
    g->debugger->z80_log_enabled = s->show_z80_log;
    g->debugger->rewinding_enabled = s->rewinding_enabled;

    if (g->status == Status_Running)
    {
        // Emulate by slices of audio sample
        a->remaining_time += dt;
        double time_slice = (double)1 / SAMPLE_RATE;

        // How many Genesis seconds we need to emulate, depending on speed factor
        double dt_genesis = time_slice * s->emulation_speed;

        while (a->remaining_time > 0)
        {
            // Emulate enough for one audio sample
            int16_t sample[2];
            genesis_run_samples(g, sample, 1, dt_genesis);

            // Queue samples to audio device
            audio_queue(a, sample, 1);

            a->remaining_time -= time_slice;

            // Exit early on breakpoint
            if (g->status != Status_Running)
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
    else if (g->status == Status_Rewinding)
    {
        if (now - last_rewind >= REWIND_PLAY_INTERVAL)
        {
            bool more_to_rewind = debugger_rewind(g->debugger);
            if (!more_to_rewind)
            {
                g->status = Status_Running;
            }

            last_rewind = now;
        }

        vdp_draw_screen(g->vdp);
    }

    video_begin_frame(app->video);

    int actions = ui_draw(app->ui, g, s, a);

    uint16_t width, height;
    uint8_t* pixels = genesis_framebuffer(g, &width, &height);
    video_draw(app->video, pixels, width, height, BUFFER_WIDTH * 3, s->video_scale);

    video_end_frame(app->video);

    apply_actions(app, actions);
    audio_set_paused(a, g->status != Status_Running);
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s ROM\n", argv[0]);
        return 1;
    }

    // Initialize

    Genesis* g = genesis_make();
    Settings* settings = settings_make();
    Audio* audio = audio_make();

    App app = {g, settings, audio, NULL, NULL};
    VideoCallbacks callbacks = {&app, on_key, on_drop, on_resize, on_close};
    app.video = video_make(settings->window_width, settings->window_height, callbacks);
    app.ui = ui_make();

    // Load the ROM

    load_rom_file(&app, argv[1]);

    // Run

    while (g->status != Status_Quitting)
    {
        update(&app);
    }

    // Save the settings on exit

    settings_save(settings);

    // Release

    ui_free(app.ui);
    video_free(app.video);
    audio_free(audio);
    settings_free(settings);
    genesis_free(g);

    return 0;
}
