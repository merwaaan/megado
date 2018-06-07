#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui/cimgui.h>
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <stdlib.h>

#include "../megado/utils.h"
#include "gym.h"
#include "sdl_imgui.h"

SDL_GLContext gl_context;
SDL_Window* window;

static void init_ui() {
  // Setup SDL
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
  SDL_DisplayMode current;
  SDL_GetCurrentDisplayMode(0, &current);
  window = SDL_CreateWindow("GYM Player",
                            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                            1280, 720, SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
  gl_context = SDL_GL_CreateContext(window);
  SDL_GL_SetSwapInterval(1); // Enable vsync

  // Setup GLEW
  printf("Initializing GLEW (%d.%d.%d)...\n", GLEW_VERSION, GLEW_VERSION_MAJOR, GLEW_VERSION_MINOR);
  GLenum glew_success = glewInit();
  if (glew_success != GLEW_OK) {
    FATAL("An error occurred while initializing GLEW: %s", glewGetErrorString(glew_success));
  }

  // Check it works
  printf("OpenGL version: %s\n", glGetString(GL_VERSION));
  glClearColor(0.45f, 0.55f, 0.65f, 1.0f);

  // Setup imGui
  sdl_imgui_init();
}

static void destroy_ui() {
  sdl_imgui_destroy();

  SDL_GL_DeleteContext(gl_context);
  SDL_DestroyWindow(window);
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

  // @Temporary: move to ui.c
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
    printf("Error initializing SDL: %s\n", SDL_GetError());
    exit(1);
  }

  init_ui();

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

  destroy_ui();
  SDL_Quit();

  return 0;
}
