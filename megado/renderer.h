#pragma once

#include <GLFW/glfw3.h>
#include <stdbool.h>
#include "vdp.h"

#define TPF_LENGTH 128

typedef struct Renderer
{
    Genesis* genesis;

    GLFWwindow* window;

    double last_time;
    float tpf[TPF_LENGTH]; // time per frame
    float avg_tpf;
    int tpf_idx;
    float tpf_refresh_counter;

    enum Planes selected_plane;
    uint8_t* plane_buffer;
    uint8_t* sprites_buffer;

    uint32_t rom_target_address;
    uint32_t ram_target_address;
    uint32_t vram_target_address;

    // Graphics resources for rendering the Genesis' output
    GLuint game_shader;
    GLuint game_texture;
    GLuint game_vertex_array_object, game_vertex_buffer_object;
    GLint game_shader_projection_loc;

    // Graphics resources for the user interface
    GLuint ui_shader;
    GLuint ui_patterns_texture, ui_magnified_pattern_texture, ui_planes_texture, ui_sprites_texture;
    GLuint ui_vertex_array_object, ui_vertex_buffer_object, ui_element_buffer_object;
    GLint ui_shader_texture_loc, ui_shader_projection_loc;

} Renderer;

Renderer* renderer_make(Genesis*);
void renderer_free(Renderer*);

void renderer_render(Renderer*);
