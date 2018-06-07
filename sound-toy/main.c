#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui/cimgui.h>
#include <GL/glew.h>

#include "../gym/sdl_imgui.h"
#include "../gym/ui.h"
#include "../megado/psg.h"
#include "../megado/ym2612.h"

static const struct ImVec4 color_title = { 0.0f, 0.68f, 0.71f, 1.0f };
static const struct ImVec4 color_dimmed = { 0.5f, 0.5f, 0.5f, 1.0f };

static const uint32_t NTSC_MASTER_FREQUENCY = 53693175;
static const uint32_t SAMPLE_RATE = 44100;

static float g_emulation_speed = 1.0f;
static YM2612 g_ym2612;
static PSG    g_psg;
static SDL_AudioDeviceID g_audio_device;

void emulate_audio() {
  static double audio_remaining_time = 0;

  // dt is wall time in seconds elapsed since last update
  static double last_update = 0;
  double now = SDL_GetTicks() / 1000.0;
  double dt = now - last_update;
  last_update = now;

  // Schedule the end of this frame from the previous frame time
  // (with some slack for overhead)
  double max_time = now + dt - (dt / 10);

  // Emulate by slices of audio sample
  audio_remaining_time += dt;
  double time_slice = (double)1 / SAMPLE_RATE;

  // How many Genesis seconds we need to emulate, depending on speed factor
  double dt_genesis = time_slice * g_emulation_speed;
  // Convert the duration to master cycles
  double d_cycles = dt_genesis * NTSC_MASTER_FREQUENCY;

  while (audio_remaining_time > 0) {
    // Emulate enough for one audio sample
    psg_run_cycles(&g_psg, d_cycles);
    ym2612_run_cycles(&g_ym2612, d_cycles);

    // Sample the audio units
    // @Temporary: use left channel for PSG and right channel for YM2612
    int16_t sample[2] = {psg_mix(&g_psg), ym2612_mix(&g_ym2612)};
    // Queue samples to audio device
    if (g_audio_device > 0 && SDL_QueueAudio(g_audio_device, sample, 2 * sizeof(int16_t)) != 0) {
      fprintf(stderr, "Failed to queue audio: %s", SDL_GetError());
    }

    audio_remaining_time -= time_slice;

    // If we are taking longer than the allocated time, abort
    if ((SDL_GetTicks() / 1000.0) > max_time) {
      break;
    }
  }
}

int main() {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
    printf("Error initializing SDL: %s\n", SDL_GetError());
    exit(1);
  }

  SDL_Window *window = init_ui();

  // Init SDL audio
  SDL_AudioSpec want, have;

  SDL_memset(&want, 0, sizeof(want));
  want.freq = SAMPLE_RATE;
  want.format = AUDIO_S16;
  want.channels = 2;
  want.samples = 512;
  want.callback = NULL; // Use SDL_QueueAudio instead

  g_audio_device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);

  if (g_audio_device == 0) {
    fprintf(stderr, "Failed to open audio: %s\n", SDL_GetError());
    exit(1);
  }

  SDL_PauseAudioDevice(g_audio_device, 0);

  psg_initialize(&g_psg);
  ym2612_initialize(&g_ym2612);

  // Main loop
  bool done = false;
  while (!done) {
    // Process events
    {
      SDL_Event event;
      while (SDL_PollEvent(&event)) {
        sdl_imgui_process_event(&event);
        if (event.type == SDL_QUIT) {
          done = true;
        }
      }
    }

    emulate_audio();

    // Render
    glClear(GL_COLOR_BUFFER_BIT);
    sdl_imgui_new_frame(window);

    // Build UI
    {
      igBegin("PSG registers", NULL, 0);

      PSG* p = &g_psg;
      int v;

      for (int i=0; i < 3; ++i) {
        igTextColored(color_title, "Square %d", i);
        if (p->square[i].volume == 0xF) {
          igSameLine(0,0);
          igTextColored(color_dimmed, " [silent]");
        }

        v = 0xF - p->square[i].volume;
        char name[10];
        sprintf(name, "volume##%d", i);
        if (igSliderInt(name, &v, 0, 0xF, NULL)) {
          p->square[i].volume = 0xF - v;
        }

        v = p->square[i].tone;
        sprintf(name, "tone##%d", i);
        if (igSliderInt(name, &v, 0, 0x3FF, NULL)) {
          p->square[i].tone = v;
        }

        igText("volume:  %0X", p->square[i].volume);
        igText("tone:    %0X [%.2fHz]", p->square[i].tone, square_tone_in_hertz(&p->square[i]));
        igText("counter: %0X", p->square[i].counter);
        igText("output:  %d", square_output(&p->square[i]));
      }

      igTextColored(color_title, "Noise");
      if (p->noise.volume == 0xF) {
        igSameLine(0,0);
        igTextColored(color_dimmed, " [silent]");
      }

      v = 0xF - p->noise.volume;
      if (igSliderInt("volume##3", &v, 0, 0xF, NULL)) {
        p->noise.volume = 0xF - v;
      }

      igText("volume:     %0X", p->noise.volume);

      bool bv = p->noise.mode;
      if (igCheckbox("mode", &bv)) {
        p->noise.mode = bv;
      }

      igText("mode:       %0X [%s]", p->noise.mode, p->noise.mode ? "white" : "periodic");

      v = p->noise.shift_rate;
      if (igSliderInt("shift rate", &v, 0, 0x3, NULL)) {
        p->noise.shift_rate = v;
      }
      igText("shift rate: %0X", p->noise.shift_rate);
      igText("counter:    %0X", p->noise.counter);

      v = p->noise.lfsr;
      if (igSliderInt("lsfr", &v, 0, 0xFFFF, NULL)) {
        p->noise.lfsr = v;
      }

      igText("lfsr:       %0X", p->noise.lfsr);
      igText("output:     %d", noise_output(&p->noise));
      igEnd();
    }

    igRender();
    SDL_GL_SwapWindow(window);
  }

  destroy_ui(window);
  SDL_Quit();

  return 0;
}
