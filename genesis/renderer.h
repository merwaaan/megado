#pragma once

#include <SDL.h>
#include <stdbool.h>

struct Vdp;
enum Planes;

typedef struct
{
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
} WindowContext;

typedef struct Renderer
{
    struct Vdp* vdp;

    WindowContext* main_window;
    WindowContext* palette_window;
    WindowContext* patterns_window;
    WindowContext* planes_window;

    enum Planes selected_plane;
} Renderer;

Renderer* renderer_make(struct Vdp*);
void renderer_free(Renderer*);

void renderer_render(Renderer*);

void renderer_toggle_palette_window(Renderer*, bool);
void renderer_toggle_patterns_window(Renderer*, bool);
void renderer_toggle_planes_window(Renderer*, bool);
void renderer_cycle_plane(Renderer*);
