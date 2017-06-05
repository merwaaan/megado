#pragma once

#include <GLFW/glfw3.h>
#include <stdbool.h>
#include "vdp.h"

typedef struct Renderer
{
    Genesis* genesis;

    GLFWwindow* window;

    bool show_cpu_registers;
    bool show_vdp_palettes;

    GLuint game_shader;
    GLuint game_texture;
    GLuint game_vertex_array_object;
    //static int g_AttribLocationProjMtx2 = 0;

    GLuint ui_shader;
    GLuint ui_vertex_array_object, ui_vertex_buffer_object, ui_element_buffer_object;
    GLint ui_shader_texture_loc, ui_shader_projection_loc;

} Renderer;

Renderer* renderer_make(Genesis*);
void renderer_free(Renderer*);

void renderer_render(Renderer*);
