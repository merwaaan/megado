#include <SDL2/SDL.h>
#include <stdbool.h>

bool sdl_imgui_process_event(SDL_Event *event);
void sdl_imgui_init();
void sdl_imgui_destroy();
void sdl_imgui_new_frame(SDL_Window *window);
