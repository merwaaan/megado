#include "renderer.h"
#include "vdp.h"

#include <stdio.h>
#include <string.h>

WindowContext* toggle_window(WindowContext* window, bool enable, int width, int height, char* title);

Renderer* renderer_make(Vdp* vdp)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        printf("An error occurred while initializing SDL: %s", SDL_GetError());
        exit(1);
    }

    Renderer* r = calloc(1, sizeof(Renderer));
    r->vdp = vdp;

    r->main_window = toggle_window(r->main_window, true, 320, 240, "Main window");

    return r;
}

void renderer_free(Renderer* r)
{
    if (r == NULL)
        return;

    toggle_window(r->main_window, false, 0, 0, "");
    renderer_toggle_palette_window(r, false);
    renderer_toggle_patterns_window(r, false);
    renderer_toggle_planes_window(r, false);

    SDL_Quit();

    free(r);
}

#define PALETTE_ENTRY_WIDTH 16

#define PATTERNS_COUNT 2048
#define PATTERNS_COLUMNS 32

#define GREYSCALE(g) { g, g, g }

// Black & white debug palette
Color debug_palette[16] = {
    GREYSCALE(0),
    GREYSCALE(17),
    GREYSCALE(34),
    GREYSCALE(45),
    GREYSCALE(68),
    GREYSCALE(85),
    GREYSCALE(106),
    GREYSCALE(119),
    GREYSCALE(136),
    GREYSCALE(153),
    GREYSCALE(170),
    GREYSCALE(187),
    GREYSCALE(204),
    GREYSCALE(221),
    GREYSCALE(238),
    GREYSCALE(255)
};

void renderer_render(Renderer* r)
{
    // Draw the video output from the VDP    
    SDL_UpdateTexture(r->main_window->texture, NULL, r->vdp->buffer, BUFFER_WIDTH * sizeof(uint8_t) * 3);
    SDL_RenderCopy(r->main_window->renderer, r->main_window->texture, NULL, NULL);
    SDL_RenderPresent(r->main_window->renderer);

    // Draw the color palette
    if (r->palette_window != NULL)
    {
        SDL_Rect cell = { 0, 0, PALETTE_ENTRY_WIDTH, PALETTE_ENTRY_WIDTH };

        for (int row = 0; row < 4; ++row)
            for (int col = 0; col < 16; ++col)
            {
                cell.x = col * PALETTE_ENTRY_WIDTH;
                cell.y = row * PALETTE_ENTRY_WIDTH;

                Color color = r->vdp->cram[row * 16 + col];
                SDL_SetRenderDrawColor(r->palette_window->renderer, color.r, color.g, color.b, 255);
                SDL_RenderFillRect(r->palette_window->renderer, &cell);
            }

        SDL_RenderPresent(r->palette_window->renderer);
    }

    // Draw the patterns
    if (r->patterns_window != NULL)
    {
        uint8_t pattern_buffer[192]; // 64 pixels * 3 bits
        SDL_Rect cell = { 0, 0, 8, 8 };

        for (int pattern = 0; pattern < PATTERNS_COUNT; ++pattern)
        {
            vdp_draw_pattern(r->vdp, pattern, debug_palette, pattern_buffer, 8, 0, 0);

            cell.x = pattern % PATTERNS_COLUMNS * 8;
            cell.y = pattern / PATTERNS_COLUMNS * 8;
            SDL_UpdateTexture(r->patterns_window->texture, &cell, pattern_buffer, 8 * 3 * sizeof(uint8_t));
        }

        SDL_RenderCopy(r->patterns_window->renderer, r->patterns_window->texture, NULL, NULL);
        SDL_RenderPresent(r->patterns_window->renderer);
    }

    // Draw the current plane
    if (r->planes_window != NULL)
    {
        // TODO do this when the plane sizes change
        //SDL_SetWindowSize(r->planes_window->window, r->vdp->horizontal_plane_size * 8, r->vdp->vertical_plane_size * 8);

        uint8_t plane_buffer[64 * 8 * 64 * 8 * 3];

        vdp_draw_plane(r->vdp, r->selected_plane, plane_buffer, 512);
        SDL_UpdateTexture(r->planes_window->texture, NULL, plane_buffer, 64 * 8 * 3 * sizeof(uint8_t));

        SDL_RenderCopy(r->planes_window->renderer, r->planes_window->texture, NULL, NULL);
        SDL_RenderPresent(r->planes_window->renderer);
    }
}

WindowContext* toggle_window(WindowContext* window, bool enable, int width, int height, char* title)
{
    if (enable && window == NULL)
    {
        window = calloc(1, sizeof(WindowContext));

        SDL_CreateWindowAndRenderer(width, height, SDL_WINDOW_OPENGL, &window->window, &window->renderer);
        SDL_SetWindowTitle(window->window, title);

        window->texture = SDL_CreateTexture(window->renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, width, height);
    }
    else if (!enable && window != NULL)
    {
        SDL_DestroyTexture(window->texture);
        SDL_DestroyRenderer(window->renderer);
        SDL_DestroyWindow(window->window);
        free(window);
        window = NULL;
    }

    return window;
}

void renderer_toggle_palette_window(Renderer* r, bool enable)
{
    r->palette_window = toggle_window(r->palette_window, enable, 16 * PALETTE_ENTRY_WIDTH, 4 * PALETTE_ENTRY_WIDTH, "Color palettes");
}

void renderer_toggle_patterns_window(Renderer* r, bool enable)
{
    r->patterns_window = toggle_window(r->patterns_window, enable, PATTERNS_COLUMNS * 8, PATTERNS_COUNT / PATTERNS_COLUMNS * 8, "Patterns");
}

void renderer_toggle_planes_window(Renderer* r, bool enable)
{
    r->planes_window = toggle_window(r->planes_window, enable, 64 * 8, 64 * 8, "Planes");
}

char* plane_names[] = { "aa", "bb", "cc" };

void renderer_cycle_plane(Renderer* r)
{
    r->selected_plane = (r->selected_plane + 1) % 3;

    if (r->planes_window != NULL)
        SDL_SetWindowTitle(r->planes_window->window, plane_names[r->selected_plane]);
}