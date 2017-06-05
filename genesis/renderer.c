#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui/cimgui.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <m68k/m68k.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "genesis.h"
#include "joypad.h"
#include "renderer.h"
#include "vdp.h"

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

GLuint create_shader(GLenum shader_type, GLchar* source)
{
    GLuint shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, &source, 0);
    glCompileShader(shader);

    GLsizei log_length;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
    if (log_length > 0)
    {
        GLchar* log = calloc(log_length, sizeof(GLchar));

        glGetShaderInfoLog(shader, log_length, NULL, log);
        printf("%s\n", log);

        free(log);
    }

    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE)
    {
        printf("Failed to compile shader\n");
        exit(1);
    }

    return shader;
}

GLuint create_shader_program(GLchar* vertex_shader_source, GLchar* fragment_shader_source)
{
    GLuint program = glCreateProgram();
    int vertex_shader = create_shader(GL_VERTEX_SHADER, vertex_shader_source);
    int fragment_shader = create_shader(GL_FRAGMENT_SHADER, fragment_shader_source);
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    GLsizei log_length;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
    if (log_length > 0)
    {
        GLchar* log = calloc(log_length, sizeof(GLchar));

        glGetProgramInfoLog(program, log_length, NULL, log);
        printf("%s\n", log);

        free(log);
    }

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success == GL_FALSE)
    {
        printf("Failed to compile shader program\n");
        exit(1);
    }

    return program;
}

GLchar* game_vertex_shader_source =
"#version 330\n"
//"uniform mat4 ProjMtx;\n"
"in vec3 vertex_position;\n"
"in vec2 vertex_texcoord;\n"
"out vec2 texcoord;\n"
"void main()\n"
"{\n"
"   texcoord = vertex_texcoord;\n"
//"	gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
"   gl_Position = vec4(vertex_position, 1.0);\n"
"}\n";

GLchar* game_fragment_shader_source =
"#version 330\n"
"uniform sampler2D sampler;\n"
"in vec2 texcoord;\n"
"out vec4 color;\n"
"void main()\n"
"{\n"
"   color = vec4(texture(sampler, texcoord).rgb, 1.0);\n"
"}\n";

void init_genesis_rendering(Renderer* r)
{
    // The Genesis' video output will be displayed on a screen-aligned quad
    float quad_extent = BUFFER_WIDTH / 800.0f;
    float quad_vertices[] = {
        // Top-right triangle
        // format: x, y, z, u, v
        -quad_extent, +quad_extent,  0.0f, 0.0f, 0.0f,
        +quad_extent, +quad_extent,  0.0f, 1.0f, 0.0f,
        +quad_extent, -quad_extent,  0.0f, 1.0f, 1.0f,

        // Bottom-let triangle
        -quad_extent, +quad_extent,  0.0f, 0.0f, 0.0f,
        +quad_extent, -quad_extent,  0.0f, 1.0f, 1.0f,
        -quad_extent, -quad_extent,  0.0f, 0.0f, 1.0f
    };

    r->game_shader = create_shader_program(game_vertex_shader_source, game_fragment_shader_source);

    GLuint vertex_buffer_object;
    glGenBuffers(1, &vertex_buffer_object);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
    glBufferData(GL_ARRAY_BUFFER, 6 * 5 * sizeof(float), quad_vertices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &r->game_vertex_array_object);
    glBindVertexArray(r->game_vertex_array_object);

    GLint shader_position_loc = glGetAttribLocation(r->game_shader, "vertex_position");
    glEnableVertexAttribArray(shader_position_loc);
    glVertexAttribPointer(shader_position_loc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);

    GLint shader_texcoord_loc = glGetAttribLocation(r->game_shader, "vertex_texcoord");
    glEnableVertexAttribArray(shader_texcoord_loc);
    glVertexAttribPointer(shader_texcoord_loc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 3 * sizeof(float));

    glGenTextures(1, &r->game_texture);
    glBindTexture(GL_TEXTURE_2D, r->game_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

const GLchar* ui_vertex_shader_source =
"#version 330\n"
"uniform mat4 ProjMtx;\n"
"in vec2 Position;\n"
"in vec2 UV;\n"
"in vec4 Color;\n"
"out vec2 Frag_UV;\n"
"out vec4 Frag_Color;\n"
"void main()\n"
"{\n"
"   Frag_UV = UV;\n"
"   Frag_Color = Color;\n"
"   gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
"}\n";

const GLchar* ui_fragment_shader_source =
"#version 330\n"
"uniform sampler2D Texture;\n"
"in vec2 Frag_UV;\n"
"in vec4 Frag_Color;\n"
"out vec4 Out_Color;\n"
"void main()\n"
"{\n"
"   Out_Color = Frag_Color * texture( Texture, Frag_UV.st);\n"
"}\n";

void init_ui_rendering(Renderer* r)
{
    struct ImGuiIO* io = igGetIO();

    // Setup the font texture

    int width, height;
    unsigned char* pixels;
    ImFontAtlas_GetTexDataAsRGBA32(io->Fonts, &pixels, &width, &height, NULL);

    int ui_font_texture;
    glGenTextures(1, &ui_font_texture);
    glBindTexture(GL_TEXTURE_2D, ui_font_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    ImFontAtlas_SetTexID(io->Fonts, ui_font_texture);

    // Setup the shaders and buffers

    r->ui_shader = create_shader_program(ui_vertex_shader_source, ui_fragment_shader_source);
    r->ui_shader_texture_loc = glGetUniformLocation(r->ui_shader, "Texture");
    r->ui_shader_projection_loc = glGetUniformLocation(r->ui_shader, "ProjMtx");

    glGenBuffers(1, &r->ui_vertex_buffer_object);
    glGenBuffers(1, &r->ui_element_buffer_object);

    glGenVertexArrays(1, &r->ui_vertex_array_object);
    glBindVertexArray(r->ui_vertex_array_object);
    glBindBuffer(GL_ARRAY_BUFFER, r->ui_vertex_buffer_object);

    GLint shader_position_loc = glGetAttribLocation(r->ui_shader, "Position");
    glEnableVertexAttribArray(shader_position_loc);
    glVertexAttribPointer(shader_position_loc, 2, GL_FLOAT, GL_FALSE, sizeof(struct ImDrawVert), (GLvoid*)0);

    GLint shader_texcoord_loc = glGetAttribLocation(r->ui_shader, "UV");
    glEnableVertexAttribArray(shader_texcoord_loc);
    glVertexAttribPointer(shader_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(struct ImDrawVert), (GLvoid*)sizeof(struct ImVec2));

    GLint shader_color_loc = glGetAttribLocation(r->ui_shader, "Color");
    glEnableVertexAttribArray(shader_color_loc);
    glVertexAttribPointer(shader_color_loc, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(struct ImDrawVert), (GLvoid*)(2 * sizeof(struct ImVec2)));
}

void render_genesis(Renderer* r)
{
    // Update the game texture with the Genesis' output
    glBindTexture(GL_TEXTURE_2D, r->game_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, BUFFER_WIDTH, BUFFER_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, r->genesis->vdp->buffer);

    glUseProgram(r->game_shader);
    glBindVertexArray(r->game_vertex_array_object);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDrawArrays(GL_TRIANGLES, 3, 3);
}

void build_ui(Renderer* r)
{
    bool dummy_flag = false;

    if (igBeginMainMenuBar())
    {
        if (igBeginMenu("CPU", true))
        {
            if (igMenuItem("Registers", NULL, r->show_cpu_registers, true))
                r->show_cpu_registers = !r->show_cpu_registers;

            if (igMenuItem("Breakpoints", NULL, dummy_flag, false))
            {
            }

            if (igMenuItem("Disassembly", NULL, dummy_flag, false))
            {
            }

            igSeparator();

            if (igMenuItem("ROM", NULL, dummy_flag, false))
            {
            }

            if (igMenuItem("RAM", NULL, dummy_flag, false))
            {
            }

            igEndMenu();
        }

        if (igBeginMenu("Video", true))
        {
            if (igMenuItem("Palettes", NULL, r->show_vdp_palettes, true))
                r->show_vdp_palettes = !r->show_vdp_palettes;

            if (igMenuItem("Patterns", NULL, dummy_flag, false))
            {
            }

            if (igMenuItem("Planes", NULL, dummy_flag, false))
            {
            }

            igSeparator();

            if (igMenuItem("VRAM", NULL, dummy_flag, false))
            {
            }

            if (igMenuItem("VSRAM", NULL, dummy_flag, false))
            {
            }

            if (igMenuItem("CRAM", NULL, dummy_flag, false))
            {
            }

            igEndMenu();
        }

        if (igBeginMenu("Audio", true))
        {
            igEndMenu();
        }

        igEndMainMenuBar();
    }

    // CPU registers
    if (r->show_cpu_registers)
    {
        igBegin("CPU registers", &r->show_cpu_registers, 0);
        igColumns(2, NULL, true);

        for (int i = 0; i < 8; ++i)
            igText("D%d %08X", i, r->genesis->m68k->data_registers[i]);

        igNextColumn();

        for (int i = 0; i < 8; ++i)
            igText("A%d %08X", i, r->genesis->m68k->address_registers[i]);

        igColumns(1, NULL, false);
        igText("Status %08X", r->genesis->m68k->status);

        bool extended = EXTENDED(r->genesis->m68k);
        bool negative = NEGATIVE(r->genesis->m68k);
        bool zero = ZERO(r->genesis->m68k);
        bool overflow = OVERFLOW(r->genesis->m68k);
        bool carry = CARRY(r->genesis->m68k);
        igCheckbox("X", &extended); igSameLine(0, 0);
        igCheckbox("N", &negative); igSameLine(0, 0);
        igCheckbox("Z", &zero); igSameLine(0, 0);
        igCheckbox("V", &overflow); igSameLine(0, 0);
        igCheckbox("C", &carry); igSameLine(0, 0);

        igEnd();
    }

    // VDP palettes
    if (r->show_vdp_palettes)
    {
        igSetNextWindowSize((struct ImVec2) { 16 * PALETTE_ENTRY_WIDTH + 50, 4 * PALETTE_ENTRY_WIDTH + 50 }, 0); // TODO it seems that the title bar and padding are counted in the height...
        igBegin("VDP palettes", &r->show_vdp_palettes, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_ShowBorders);

        struct ImDrawList* draw_list = igGetWindowDrawList();

        struct ImVec2 window_position;
        igGetCursorScreenPos(&window_position);

        struct ImVec2 cell_top_left, cell_bottom_right;

        for (int row = 0; row < 4; ++row)
            for (int col = 0; col < 16; ++col)
            {
                cell_top_left.x = window_position.x + col * PALETTE_ENTRY_WIDTH;
                cell_top_left.y = window_position.y + row * PALETTE_ENTRY_WIDTH;
                cell_bottom_right.x = cell_top_left.x + PALETTE_ENTRY_WIDTH;
                cell_bottom_right.y = cell_top_left.y + PALETTE_ENTRY_WIDTH;

                Color color = r->genesis->vdp->cram[row * 16 + col];
                ImDrawList_AddRectFilled(draw_list, cell_top_left, cell_bottom_right, 255 << 24 | color.b << 16 | color.g << 8 | color.r, 0, 0);
            }

        igEnd();
    }

    bool a = true;
    igShowTestWindow(&a);
}

void render_ui(Renderer* r)
{
    struct ImGuiIO* io = igGetIO();

    // Setup the display size (every frame to accommodate for window resizing)
    int window_width, window_height, display_width, display_height;
    glfwGetWindowSize(r->window, &window_width, &window_height);
    glfwGetFramebufferSize(r->window, &display_width, &display_height);
    io->DisplaySize = (struct ImVec2) { (float)window_width, (float)window_height };
    io->DisplayFramebufferScale = (struct ImVec2) { window_width > 0 ? ((float)display_width / window_width) : 0, window_height > 0 ? ((float)display_height / window_height) : 0 };

    igNewFrame();
    build_ui(r);
    igRender();

    //draw_data->ScaleClipRects(io->DisplayFramebufferScale);

    // Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);

    glUseProgram(r->ui_shader);

    int fb_width = (int)(io->DisplaySize.x * io->DisplayFramebufferScale.x);
    int fb_height = (int)(io->DisplaySize.y * io->DisplayFramebufferScale.y);
    glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);

    const float ortho_projection[4][4] =
    {
        { 2.0f / io->DisplaySize.x, 0.0f,                   0.0f, 0.0f },
        { 0.0f,                  2.0f / -io->DisplaySize.y, 0.0f, 0.0f },
        { 0.0f,                  0.0f,                  -1.0f, 0.0f },
        { -1.0f,                  1.0f,                   0.0f, 1.0f },
    };

    glUniformMatrix4fv(r->ui_shader_projection_loc, 1, GL_FALSE, &ortho_projection[0][0]);

    glBindVertexArray(r->ui_vertex_array_object);

    struct ImDrawData* draw_data = igGetDrawData();
    for (int n = 0; n < draw_data->CmdListsCount; ++n)
    {
        struct ImDrawList* commands = draw_data->CmdLists[n];
        ImDrawIdx* idx_buffer_offset = 0;

        int command_count = ImDrawList_GetCmdSize(commands);
        int vertex_count = ImDrawList_GetVertexBufferSize(commands);
        int index_count = ImDrawList_GetIndexBufferSize(commands);

        glBindBuffer(GL_ARRAY_BUFFER, r->ui_vertex_buffer_object);
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)vertex_count * sizeof(struct ImDrawVert), (GLvoid*)ImDrawList_GetVertexPtr(commands, 0), GL_STREAM_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r->ui_element_buffer_object);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)index_count * sizeof(ImDrawIdx), (GLvoid*)ImDrawList_GetIndexPtr(commands, 0), GL_STREAM_DRAW);

        for (int i = 0; i < command_count; ++i)
        {
            struct ImDrawCmd* command = ImDrawList_GetCmdPtr(commands, i);

            glBindTexture(GL_TEXTURE_2D, (GLuint)command->TextureId);
            glScissor((int)command->ClipRect.x, (int)(fb_height - command->ClipRect.w), (int)(command->ClipRect.z - command->ClipRect.x), (int)(command->ClipRect.w - command->ClipRect.y));
            glDrawElements(GL_TRIANGLES, (GLsizei)command->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, (GLvoid*)idx_buffer_offset);

            idx_buffer_offset += command->ElemCount;
        }
    }

    glDisable(GL_SCISSOR_TEST);
}

void handle_inputs(Renderer* r, int key, int action)
{
    switch (action)
    {
    case GLFW_PRESS:
        switch (key)
        {
        case GLFW_KEY_LEFT: joypad_press(r->genesis->joypad, Left); break;
        case GLFW_KEY_RIGHT: joypad_press(r->genesis->joypad, Right); break;
        case GLFW_KEY_UP: joypad_press(r->genesis->joypad, Up); break;
        case GLFW_KEY_DOWN: joypad_press(r->genesis->joypad, Down); break;
        case GLFW_KEY_ENTER: joypad_press(r->genesis->joypad, Start); break;
        case GLFW_KEY_Q: joypad_press(r->genesis->joypad, ButtonA); break;
        case GLFW_KEY_W: joypad_press(r->genesis->joypad, ButtonB); break;
        case GLFW_KEY_E: joypad_press(r->genesis->joypad, ButtonC); break;
        }
        break;
    case GLFW_RELEASE:
        switch (key)
        {
        case GLFW_KEY_LEFT: joypad_release(r->genesis->joypad, Left); break;
        case GLFW_KEY_RIGHT: joypad_release(r->genesis->joypad, Right); break;
        case GLFW_KEY_UP: joypad_release(r->genesis->joypad, Up); break;
        case GLFW_KEY_DOWN: joypad_release(r->genesis->joypad, Down); break;
        case GLFW_KEY_ENTER: joypad_release(r->genesis->joypad, Start); break;
        case GLFW_KEY_Q: joypad_release(r->genesis->joypad, ButtonA); break;
        case GLFW_KEY_W: joypad_release(r->genesis->joypad, ButtonB); break;
        case GLFW_KEY_E: joypad_release(r->genesis->joypad, ButtonC); break;
        }
        break;
    }
}

void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int modifiers)
{
    Renderer* r = (Renderer*)glfwGetWindowUserPointer(window);
    handle_inputs(r, key, modifiers);

    // Pass the event to the UI

    struct ImGuiIO* io = igGetIO();
    if (action == GLFW_PRESS)
        io->KeysDown[key] = true;
    else if (action == GLFW_RELEASE)
        io->KeysDown[key] = false;

    io->KeyCtrl = io->KeysDown[GLFW_KEY_LEFT_CONTROL] || io->KeysDown[GLFW_KEY_RIGHT_CONTROL];
    io->KeyShift = io->KeysDown[GLFW_KEY_LEFT_SHIFT] || io->KeysDown[GLFW_KEY_RIGHT_SHIFT];
    io->KeyAlt = io->KeysDown[GLFW_KEY_LEFT_ALT] || io->KeysDown[GLFW_KEY_RIGHT_ALT];
    io->KeySuper = io->KeysDown[GLFW_KEY_LEFT_SUPER] || io->KeysDown[GLFW_KEY_RIGHT_SUPER];
}

static void mouse_move_callback(GLFWwindow* window, double x, double y)
{
    igGetIO()->MousePos = (struct ImVec2) { (float)x, (float)y };
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int modifiers)
{
    igGetIO()->MouseDown[button] = action == GLFW_PRESS;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    igGetIO()->MouseWheel += (float)yoffset;
}

Renderer* renderer_make(Genesis* genesis)
{
    printf("Initializing GLFW (%s)...\n", glfwGetVersionString());

    int glfw_success = glfwInit();
    if (glfw_success != GLFW_TRUE)
    {
        printf("An error occurred while initializing GLEW"); // TODO error message
        exit(1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 800, "Megado", NULL, NULL);
    if (!window)
    {
        printf("An error occurred while creating a GLEW window");
        exit(1);
    }

    glfwSetKeyCallback(window, keyboard_callback);
    glfwSetCursorPosCallback(window, mouse_move_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwMakeContextCurrent(window);

    printf("Initializing GLEW (%d.%d.%d)...\n", GLEW_VERSION, GLEW_VERSION_MAJOR, GLEW_VERSION_MINOR);

    GLenum glew_success = glewInit();
    if (glew_success != GLEW_OK)
    {
        printf("An error occurred while initializing GLEW: %s\n", glewGetErrorString(glew_success));
        exit(1);
    }

    char* gl_version;
    gl_version = (char*)glGetString(GL_VERSION);
    printf("OpenGL version: %s\n", gl_version);

    Renderer* r = calloc(1, sizeof(Renderer));
    r->genesis = genesis;
    r->window = window;

    // Store a pointer to the renderer in the window so that it can be accessed from callback functions
    glfwSetWindowUserPointer(r->window, r);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    init_ui_rendering(r);
    init_genesis_rendering(r);

    return r;
}

void renderer_free(Renderer* r)
{
    if (r == NULL)
        return;

    glfwDestroyWindow(r->window);
    glfwTerminate();

    free(r);
}

void renderer_render(Renderer* r)
{
    glClear(GL_COLOR_BUFFER_BIT);

    render_genesis(r);
    render_ui(r);

    glfwSwapBuffers(r->window);
    glfwPollEvents();

    /*
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
        }*/
}
