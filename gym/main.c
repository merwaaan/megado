#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui/cimgui.h>
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <pthread.h>
#include <stdlib.h>

#include "gym.h"
#include "sdl_imgui.h"
#include "ui.h"

static const struct ImVec4 color_title = { 0.0f, 0.68f, 0.71f, 1.0f };
static const struct ImVec4 color_dimmed = { 0.5f, 0.5f, 0.5f, 1.0f };

void *start_playing(void *arg) {
  struct GYM *gym = (struct GYM*) arg;
  gym_play(*gym);
  return NULL;
}

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: gym GYM_FILE");
    exit(1);
  }

  char *gym_file_path = argv[1];
  FILE *gym_file;

  if ((gym_file = fopen(gym_file_path, "rb")) == NULL) {
    perror("Error opening GYM file");
    exit(1);
  }

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
    printf("Error initializing SDL: %s\n", SDL_GetError());
    exit(1);
  }

  SDL_Window *window = init_ui();

  //gym_play(read_gym(gym_file));
  struct GYM gym = read_gym(gym_file);

  bool play_request = false;
  pthread_t play_thread = 0;

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

    // Render
    glClear(GL_COLOR_BUFFER_BIT);
    sdl_imgui_new_frame(window);

    {
      igBegin("Song", NULL, 0);

      if (igCheckbox("playing", &play_request)) {
        if (play_request && !play_thread) {
          pthread_create(&play_thread, NULL, start_playing, &gym);
        } else if (!play_request && play_thread) {
          gym_stop();
          play_thread = 0;
        }
      }

      igText("song       : %s\n", gym.header->song);
      igText("game       : %s\n", gym.header->game);
      igText("copyright  : %s\n", gym.header->copyright);
      igText("emulator   : %s\n", gym.header->emulator);
      igText("dumper     : %s\n", gym.header->dumper);
      igText("comment    : %s\n", gym.header->comment);
      igText("loop_start : %u\n", gym.header->loop_start);

      igEnd();
    }

    if (gym_psg) {
      igBegin("PSG registers", NULL, 0);

      PSG* p = gym_psg;

      for (int i=0; i < 3; ++i) {
        igTextColored(color_title, "Square %d", i);
        if (p->square[i].volume == 0xF) {
          igSameLine(0,0);
          igTextColored(color_dimmed, " [silent]");
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
      igText("volume:     %0X", p->noise.volume);
      igText("mode:       %0X [%s]", p->noise.mode, p->noise.mode ? "white" : "periodic");
      igText("shift rate: %0X", p->noise.shift_rate);
      igText("counter:    %0X", p->noise.counter);
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
