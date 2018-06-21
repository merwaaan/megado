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

static double audio_remaining_time = 0;

void emulate_audio() {
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
      igBegin("Metrics", NULL, 0);

      igTextColored(color_title, "Remaining audio time: ");
      igSameLine(0,0);
      igText("%.6fms", audio_remaining_time * 1000);

      igTextColored(color_title, "Remaining master cycles");
      igText("PSG:    %d", g_psg.remaining_master_cycles);
      igText("YM2612: %d", g_ym2612.remaining_master_cycles);

      igEnd();
    }

    {
      igBegin("PSG", NULL, 0);

      PSG* p = &g_psg;
      int v;

      for (int i=0; i < 3; ++i) {
        igPushIdInt(i);

        igTextColored(color_title, "Square %d", i);
        if (p->square[i].volume == 0xF) {
          igSameLine(0,0);
          igTextColored(color_dimmed, " [silent]");
        }

        v = 0xF - p->square[i].volume;
        if (igSliderInt("volume", &v, 0, 0xF, NULL)) {
          p->square[i].volume = 0xF - v;
        }

        v = 0x3FF - p->square[i].tone;
        if (igSliderInt("tone", &v, 0, 0x3FF, NULL)) {
          p->square[i].tone = 0x3FF - v;
        }
        igSameLine(0,0);
        igText(" [%.2fHz]", square_tone_in_hertz(&p->square[i]));

        igPopId();
      }

      igTextColored(color_title, "Noise");
      if (p->noise.volume == 0xF) {
        igSameLine(0,0);
        igTextColored(color_dimmed, " [silent]");
      }

      v = 0xF - p->noise.volume;
      if (igSliderInt("volume##noise", &v, 0, 0xF, NULL)) {
        p->noise.volume = 0xF - v;
      }

      bool bv = p->noise.mode;
      if (igCheckbox("mode", &bv)) {
        p->noise.mode = bv;
      }

      igSameLine(0, 0);

      igText(" [%s]", p->noise.mode ? "white" : "periodic");

      v = p->noise.shift_rate;
      if (igSliderInt("shift rate", &v, 0, 0x3, NULL)) {
        p->noise.shift_rate = v;
      }

      v = p->noise.lfsr;
      if (igSliderInt("lsfr", &v, 0, 0xFFFF, NULL)) {
        p->noise.lfsr = v;
      }

      igEnd();
    }

    {
      igBegin("YM2612", NULL, 0);

      YM2612* y = &g_ym2612;
      int v;
      bool bv;

      igColumns(7, NULL, false);

      igTextColored(color_title, "Channel");
      igText("muted");
      igText("enabled");
      igText("freq block");
      igText("freq number");
      igText("frequency");
      igText("algorithm");

      igNextColumn();

      // Channels
      for (int i=0; i < 6; ++i) {
        igPushIdInt(i);

        igTextColored(color_title, "%d", i + 1);

        if ((i+1) % 3 == 0) {
          uint8_t channel_mode = i == 3 ? y->channel3_mode : y->channel6_mode;

          char* mode_string = "normal";
          switch (channel_mode) {
          case 0: mode_string = "normal"; break;
          case 1: mode_string = "special"; break;
          default: mode_string = "illegal"; break;
          }

          igSameLine(0,0);
          igText(" (%s)", mode_string);
        }

        // FIXME: the checkbox and sliders are too tall for the text, shifting
        // all the lines in the table after it.
        igCheckbox("##y.muted", &y->channels[i].muted);

        if (igSmallButton("Key on")) {
          y->channels[i].enabled = true;
          for (int j=0; j < 4; ++j) {
            Operator* op = &y->channels[i].operators[j];
            op->adsr_phase = ATTACK;
            op->attenuation = 0x3ff;
            y->envelope_counter = 0;
          }
        }

        if (igSmallButton("Key off")) {
          y->channels[i].enabled = false;
          for (int j=0; j < 4; ++j) {
            Operator* op = &y->channels[i].operators[j];
            op->adsr_phase = RELEASE;
          }
        }

        v = y->channels[i].frequency.block;
        if (igSliderInt("##freq.block", &v, 0, 0x7, NULL)) {
          y->channels[i].frequency.block = v;
        }

        v = y->channels[i].frequency.freq;
        if (igSliderInt("##freq.number", &v, 0, 0x7FF, NULL)) {
          y->channels[i].frequency.freq = v;
        }

        igText("%.2fHz", channel_frequency_in_hertz(&y->channels[i], NTSC_MASTER_FREQUENCY));

        v = y->channels[i].algorithm;
        if (igSliderInt("##algorithm", &v, 0, 0x7, NULL)) {
          y->channels[i].algorithm = v;
        }

        igNextColumn();

        igPopId();
      }

      igSeparator();

      // Operators
      for (int i=0; i < 6; ++i) {
        igPushIdInt(i);
        igColumns(1, NULL, false);
        if (igTreeNodePtr((void*)(intptr_t)i, "Operators for channel %d", i+1)) {

          igColumns(5, NULL, false);

          igTextColored(color_title, "Operator");
          igText("detune");
          igText("multiple");
          /* igText("total level"); */
          /* igText("attack rate"); */
          /* igText("decay rate"); */
          /* igText("sustain level"); */
          /* igText("sustain rate"); */
          /* igText("release rate"); */
          /* igText("rate scaling"); */
          /* igText("amp modulation"); */

          igNextColumn();

          for (int j=0; j < 4; ++j) {
            igPushIdInt(j);
            igTextColored(color_title, "%d", j + 1);

            v = y->channels[i].operators[j].detune;
            if (igSliderInt("##detune", &v, 0, 0x7, NULL)) {
              y->channels[i].operators[j].detune = v;
            }

            v = y->channels[i].operators[j].multiple;
            if (igSliderInt("##multiple", &v, 0, 0xF, NULL)) {
              y->channels[i].operators[j].multiple = v;
            }

            /* v = y->channels[i].operators[j].total_level; */
            /* if (igSliderInt("##total_level", &v, 0, 0x7F, NULL)) { */
            /*   y->channels[i].operators[j].total_level = v; */
            /* } */

            /* v = y->channels[i].operators[j].attack_rate; */
            /* if (igSliderInt("##attack_rate", &v, 0, 0x1F, NULL)) { */
            /*   y->channels[i].operators[j].attack_rate = v; */
            /* } */

            /* v = y->channels[i].operators[j].decay_rate; */
            /* if (igSliderInt("##decay_rate", &v, 0, 0x1F, NULL)) { */
            /*   y->channels[i].operators[j].decay_rate = v; */
            /* } */

            /* v = y->channels[i].operators[j].sustain_level; */
            /* if (igSliderInt("##sustain_level", &v, 0, 0xF, NULL)) { */
            /*   y->channels[i].operators[j].sustain_level = v; */
            /* } */

            /* v = y->channels[i].operators[j].sustain_rate; */
            /* if (igSliderInt("##sustain_rate", &v, 0, 0x1F, NULL)) { */
            /*   y->channels[i].operators[j].sustain_rate = v; */
            /* } */

            /* v = y->channels[i].operators[j].release_rate; */
            /* if (igSliderInt("##release_rate", &v, 0, 0xF, NULL)) { */
            /*   y->channels[i].operators[j].release_rate = v; */
            /* } */

            /* v = y->channels[i].operators[j].rate_scaling; */
            /* if (igSliderInt("##rate_scaling", &v, 0, 0x3, NULL)) { */
            /*   y->channels[i].operators[j].rate_scaling = v; */
            /* } */

            /* bv = y->channels[i].operators[j].amplitude_modulation_enabled; */
            /* if (igCheckbox("##amplitude_modulation_enabled", &bv)) { */
            /*   y->channels[i].operators[j].amplitude_modulation_enabled = bv; */
            /* } */

            igNextColumn();
            igPopId();
          }

          igTreePop();
        }
        igPopId();
      }

      igEnd();
    }

    igRender();
    SDL_GL_SwapWindow(window);
  }

  destroy_ui(window);
  SDL_Quit();

  return 0;
}
