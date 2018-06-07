#include <cimgui/cimgui.h>
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <stdlib.h>

#include "gym.h"
#include "sdl_imgui.h"
#include "ui.h"

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

    igBegin("Song", NULL, 0);

    igText("song       : %s\n", gym.header->song);
    igText("game       : %s\n", gym.header->game);
    igText("copyright  : %s\n", gym.header->copyright);
    igText("emulator   : %s\n", gym.header->emulator);
    igText("dumper     : %s\n", gym.header->dumper);
    igText("comment    : %s\n", gym.header->comment);
    igText("loop_start : %u\n", gym.header->loop_start);

    igEnd();

    igRender();
    SDL_GL_SwapWindow(window);
  }

  destroy_ui(window);
  SDL_Quit();

  return 0;
}
