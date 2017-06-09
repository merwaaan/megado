#pragma once

#include <GLFW/glfw3.h>
#include <stdbool.h>
#include "vdp.h"

enum Planes;

typedef struct Renderer
{
    Genesis* genesis;

    GLFWwindow* window;

    bool show_cpu_registers;
    bool show_vdp_palettes;
    bool show_vdp_patterns;
    bool show_cpu_disassembly;

    bool show_rom;
    uint32_t rom_target_address;

    bool show_ram;
    uint32_t ram_target_address;

    bool show_vdp_planes;
    enum Planes selected_plane;
    uint8_t* plane_buffer;

    bool show_vram;
    uint32_t vram_target_address;

    bool show_vsram;
    bool show_cram;

    // Graphics resources for rendering the Genesis' output
    GLuint game_shader;
    GLuint game_texture;
    GLuint game_vertex_array_object, game_vertex_buffer_object;
    GLint game_shader_projection_loc;

    // Graphics resources for the user interface
    GLuint ui_shader;
    GLuint ui_patterns_texture, ui_planes_texture;
    GLuint ui_vertex_array_object, ui_vertex_buffer_object, ui_element_buffer_object;
    GLint ui_shader_texture_loc, ui_shader_projection_loc;

} Renderer;

Renderer* renderer_make(Genesis*);
void renderer_free(Renderer*);

void renderer_render(Renderer*);
