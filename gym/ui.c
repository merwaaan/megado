#include <GL/glew.h>

#include "../megado/utils.h"
#include "sdl_imgui.h"

static SDL_GLContext gl_context;

SDL_Window* init_ui() {
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
  SDL_Window *window = SDL_CreateWindow("GYM Player",
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

  return window;
}

void destroy_ui(SDL_Window* window) {
  sdl_imgui_destroy();

  SDL_GL_DeleteContext(gl_context);
  SDL_DestroyWindow(window);
}
