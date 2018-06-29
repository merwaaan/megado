#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cimgui/cimgui.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "audio.h"
#include "genesis.h"
#include "joypad.h"
#include "metric.h"
#include "psg.h"
#include "renderer.h"
#include "settings.h"
#include "utils.h"
#include "ym2612.h"

#define DISASSEMBLY_LENGTH 10
#define MEMORY_VIEWER_COLUMNS 16

#define PALETTE_ENTRY_WIDTH 16
#define PATTERNS_COUNT 2048
#define PATTERNS_COLUMNS 32
#define PATTERN_MAGNIFICATION_FACTOR 16

#define GREYSCALE(g) { g, g, g }

// Black & white debug palette
static Color debug_palette[16] = {
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

static GLuint create_shader(GLenum shader_type, const GLchar* source)
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
        FATAL("Failed to compile shader");
    }

    return shader;
}

static GLuint create_shader_program(const GLchar* vertex_shader_source, const GLchar* fragment_shader_source)
{
    GLuint program = glCreateProgram();
    GLuint vertex_shader = create_shader(GL_VERTEX_SHADER, vertex_shader_source);
    GLuint fragment_shader = create_shader(GL_FRAGMENT_SHADER, fragment_shader_source);
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    glDetachShader(program, vertex_shader);
    glDetachShader(program, fragment_shader);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

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
        FATAL("Failed to compile shader program");
    }

    return program;
}

static const GLchar* game_vertex_shader_source =
"#version 120\n"
"uniform mat4 projection_matrix;\n"
"attribute vec3 vertex_position;\n"
"attribute vec2 vertex_texcoord;\n"
"varying vec2 texcoord;\n"
"void main()\n"
"{\n"
"   texcoord = vertex_texcoord;\n"
"   gl_Position = projection_matrix * vec4(vertex_position, 1.0);\n"
"}\n";

static const GLchar* game_fragment_shader_source =
"#version 120\n"
"uniform sampler2D sampler;\n"
"varying vec2 texcoord;\n"
"void main()\n"
"{\n"
"   gl_FragColor = vec4(texture2D(sampler, texcoord).rgb, 1.0);\n"
"}\n";

static void init_genesis_rendering(Renderer* r)
{
    glGenTextures(1, &r->game_texture);
    glBindTexture(GL_TEXTURE_2D, r->game_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Setup the shader and buffers

    r->game_shader = create_shader_program(game_vertex_shader_source, game_fragment_shader_source);

    r->game_shader_projection_loc = glGetUniformLocation(r->game_shader, "projection_matrix");

    glGenBuffers(1, &r->game_vertex_buffer_object);
    glBindBuffer(GL_ARRAY_BUFFER, r->game_vertex_buffer_object);

    glGenVertexArrays(1, &r->game_vertex_array_object);
    glBindVertexArray(r->game_vertex_array_object);

    GLint shader_position_loc = glGetAttribLocation(r->game_shader, "vertex_position");
    glEnableVertexAttribArray(shader_position_loc);
    glVertexAttribPointer(shader_position_loc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);

    GLint shader_texcoord_loc = glGetAttribLocation(r->game_shader, "vertex_texcoord");
    glEnableVertexAttribArray(shader_texcoord_loc);
    glVertexAttribPointer(shader_texcoord_loc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)(3 * sizeof(float)));
}

static const GLchar* ui_vertex_shader_source =
"#version 120\n"
"uniform mat4 ProjMtx;\n"
"attribute vec2 Position;\n"
"attribute vec2 UV;\n"
"attribute vec4 Color;\n"
"varying vec2 Frag_UV;\n"
"varying vec4 Frag_Color;\n"
"void main()\n"
"{\n"
"   Frag_UV = UV;\n"
"   Frag_Color = Color;\n"
"   gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
"}\n";

static const GLchar* ui_fragment_shader_source =
"#version 120\n"
"uniform sampler2D Texture;\n"
"varying vec2 Frag_UV;\n"
"varying vec4 Frag_Color;\n"
"void main()\n"
"{\n"
"   gl_FragColor = Frag_Color * texture2D( Texture, Frag_UV.st);\n"
"}\n";

static void gen_texture(GLuint* texture)
{
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

static void init_ui_rendering(Renderer* r)
{
    struct ImGuiIO* io = igGetIO();

    igPushStyleVar(ImGuiStyleVar_WindowRounding, 0);

    // Setup the font texture

    int width, height;
    unsigned char* pixels;
    ImFontAtlas_GetTexDataAsRGBA32(io->Fonts, &pixels, &width, &height, NULL);

    GLuint ui_font_texture;
    glGenTextures(1, &ui_font_texture);
    glBindTexture(GL_TEXTURE_2D, ui_font_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    ImFontAtlas_SetTexID(io->Fonts, (ImTextureID)(intptr_t)ui_font_texture);

    // Setup texture for the patterns and planes debug views
    gen_texture(&r->ui_patterns_texture);
    gen_texture(&r->ui_magnified_pattern_texture);
    gen_texture(&r->ui_planes_texture);
    gen_texture(&r->ui_sprites_texture);

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

static void render_genesis(Renderer* r)
{
    // Update the game texture with the Genesis' output

    uint16_t output_width, output_height;
    vdp_get_resolution(r->genesis->vdp, &output_width, &output_height);

    glBindTexture(GL_TEXTURE_2D, r->game_texture);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, BUFFER_WIDTH); // Whatever the current width of the output, be sure to consider the buffer full width (in low resolution mode, the rightmost pixels will be skipped)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, output_width, output_height, 0, GL_RGB, GL_UNSIGNED_BYTE, r->genesis->vdp->output_buffer);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

    // The Genesis' video output will be displayed on the following screen-aligned quad

    int display_width, display_height;
    glfwGetWindowSize(r->window, &display_width, &display_height);
    glViewport(0, 0, display_width, display_height);

    float display_center_x = display_width / 2.0f;
    float display_center_y = display_height / 2.0f;

    float output_half_width = output_width / 2.0f * r->genesis->settings->video_scale;
    float output_half_height = output_height / 2.0f * r->genesis->settings->video_scale;

    float quad_vertices[] = {
        // Top-right triangle
        // format: x, y, z, u, v
        display_center_x - output_half_width, display_center_y - output_half_height,  0.0f, 0.0f, 0.0f,
        display_center_x + output_half_width, display_center_y - output_half_height,  0.0f, 1.0f, 0.0f,
        display_center_x + output_half_width, display_center_y + output_half_height,  0.0f, 1.0f, 1.0f,

        // Bottom-let triangle
        display_center_x - output_half_width, display_center_y - output_half_height,  0.0f, 0.0f, 0.0f,
        display_center_x + output_half_width, display_center_y + output_half_height,  0.0f, 1.0f, 1.0f,
        display_center_x - output_half_width, display_center_y + output_half_height,  0.0f, 0.0f, 1.0f
    };

    glBindBuffer(GL_ARRAY_BUFFER, r->game_vertex_buffer_object);
    glBufferData(GL_ARRAY_BUFFER, 30 * sizeof(float), quad_vertices, GL_DYNAMIC_DRAW);

    // Update the projection matrix depending on the window size

    const GLfloat ortho_projection[4][4] =
    {
        { 2.0f / display_width, 0.0f,                   0.0f, 0.0f },
        { 0.0f,                 2.0f / -display_height, 0.0f, 0.0f },
        { 0.0f,                 0.0f,                   -1.0f, 0.0f },
        { -1.0f,                1.0f,                   0.0f, 1.0f },
    };

    glUseProgram(r->game_shader);
    glUniformMatrix4fv(r->game_shader_projection_loc, 1, GL_FALSE, &ortho_projection[0][0]);

    // Draw the quad
    glBindVertexArray(r->game_vertex_array_object);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDrawArrays(GL_TRIANGLES, 3, 3);
}

static const struct ImVec4 color_black = { 0.0f, 0.0f, 0.0f, 1.0f };
static const struct ImVec4 color_white = { 1.0f, 1.0f, 1.0f, 1.0f };
static const struct ImVec4 color_dimmed = { 0.5f, 0.5f, 0.5f, 1.0f };
static const struct ImVec4 color_disabled = { 0.25f, 0.25f, 0.25f, 1.0f };
static const struct ImVec4 color_accent = { 1.0f, 0.07f, 0.57f, 1.0f };
static const struct ImVec4 color_success = { 0.0f, 1.0f, 0.0f, 1.0f };
static const struct ImVec4 color_error = { 1.0f, 0.0f, 0.0f, 1.0f };
static const struct ImVec4 color_title = { 0.0f, 0.68f, 0.71f, 1.0f };

static const struct ImVec2 vec_zero = { 0.0f, 0.0f };
static const struct ImVec2 vec_one = { 1.0f, 1.0f };

static void memory_viewer(char* name, bool* opened, void* data, Size data_size, uint32_t data_length, uint32_t* target_address)
{
    //igSetNextWindowSize(vec_zero, 0);
    if (igBegin(name, opened, 0))
    {
        igBeginChild("##memory", (const struct ImVec2) { 400, 300 }, false, 0); // TODO approximate sizing, not sure how to cleanly make the child fit

        // We use a list clipper to browse the whole memory
        // while rendering only its visible section
        struct ImGuiListClipper list_clipper;
        ImGuiListClipper_Begin(&list_clipper, data_length / MEMORY_VIEWER_COLUMNS, igGetTextLineHeight());

        int first_visible_row = ImGuiListClipper_GetDisplayStart(&list_clipper);
        int last_visible_row = ImGuiListClipper_GetDisplayEnd(&list_clipper);

        for (int row = first_visible_row; row < last_visible_row; ++row)
        {
            // Count digits for the address
            int addr_digits = 0;
            for (int i = data_length - 1; i > 0; i >>= 4)
                ++addr_digits;

            uint32_t row_address = row * MEMORY_VIEWER_COLUMNS;
            igText("%0*X:  ", addr_digits, row_address);

            for (int column = 0; column < MEMORY_VIEWER_COLUMNS; ++column)
            {
                igSameLine(0, 0);

                if (data_size == Byte)
                    igText("%02X ", ((uint8_t*)data)[row_address + column]);
                else
                    igText("%04X ", ((uint16_t*)data)[row_address + column]);
            }
        }

        ImGuiListClipper_End(&list_clipper);
        igEndChild();

        if (target_address != NULL)
        {
            igSeparator();

            igAlignFirstTextHeightToWidgets();
            igText("Go to address");
            igSameLine(0, 10);
            if (igInputInt("##address", (int*)target_address, 16, 32, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_EnterReturnsTrue))
            {
                // Re-enter the context of the scrolling area
                igBeginChild("##memory", (const struct ImVec2) { -1, -1 }, false, 0);

                // Scroll to the appropriate row
                struct ImVec2 cursor_start_position;
                igGetCursorStartPos(&cursor_start_position);
                igSetScrollFromPosY(cursor_start_position.y + (*target_address / MEMORY_VIEWER_COLUMNS) * igGetTextLineHeight(), 0);

                igEndChild();
            }
        }
    }
    igEnd();
}

static void toggle_pause(Renderer* r)
{
    if (r->genesis->status == Status_Running)
        r->genesis->status = Status_Pause;
    else if (r->genesis->status == Status_Pause)
        r->genesis->status = Status_Running;
}

static void toggle_full_screen(Renderer* r)
{
    r->genesis->settings->full_screen = !r->genesis->settings->full_screen;

    if (r->genesis->settings->full_screen)
    {
        // Save the current size to restore it later
        glfwGetWindowSize(r->window, &r->window_previous_width, &r->window_previous_height);

        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        glfwSetWindowMonitor(r->window, monitor, 0, 0, mode->width, mode->height, GLFW_DONT_CARE);
    }
    else
    {
        glfwSetWindowMonitor(r->window, NULL, 50, 50, r->window_previous_width, r->window_previous_height, GLFW_DONT_CARE);
    }
}

static void toggle_vsync(Renderer* r) {
    r->genesis->settings->vsync = !r->genesis->settings->vsync;
    glfwSwapInterval(r->genesis->settings->vsync ? 1 : 0);
}

static void step(Renderer* r)
{
    if (r->genesis->status == Status_Pause)
        genesis_step(r->genesis);
}

// Return the cursor's position in screen space
static struct ImVec2 get_cursor()
{
    struct ImVec2 window, content;
    igGetWindowPos(&window);
    igGetCursorStartPos(&content);

    return (struct ImVec2) { window.x + content.x, window.y + content.y };
}

// Return the mouse position in the current window's space
static struct ImVec2 get_mouse_wrt_window()
{
    struct ImVec2 mouse, window, content;
    igGetMousePos(&mouse);
    igGetWindowPos(&window);
    igGetCursorStartPos(&content);

    return (struct ImVec2) { mouse.x - (window.x + content.x), mouse.y - (window.y + content.y) };
}

static void text_on_off(bool state, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    igTextV(format, args);
    va_end(args);

    igSameLine(0, 5);
    igTextColored(state ? color_accent : color_dimmed, state ? "on" : "off");
}

static void build_ui(Renderer* r)
{
    Settings* settings = r->genesis->settings;

    // TODO wrapp all the igBegin in if, otherwise collapsed windows still render

    bool dummy_flag = false;

    if (igBeginMainMenuBar())
    {
        if (igBeginMenu("Game", true))
        {
            // Snapshots
            if (igBeginMenu("Save slots", true))
            {
                for (uint8_t slot = 0; slot < SNAPSHOT_SLOTS; ++slot)
                {
                    igText("Slot %d:", slot);
                    igSameLine(0, 5);

                    if (r->snapshots[slot] == NULL)
                    {
                        igTextColored(color_dimmed, "       empty       ");
                    }
                    else
                    {
                        char date[30];
                        strftime(date, 30, "%Y-%m-%d %H:%M:%S", localtime(&r->snapshots[slot]->date));
                        igTextColored(color_accent, "%s", date);
                    }

                    igSameLine(0, 10);

                    char button_name[10];
                    sprintf(button_name, "save##%d", slot);
                    if (igButton(button_name, (struct ImVec2) { 50, 20 }))
                        r->snapshots[slot] = snapshot_save(r->genesis, slot);

                    igSameLine(0, 3);

                    if (r->snapshots[slot] == NULL)
                    {
                        igPushStyleColor(ImGuiCol_Button, color_disabled);
                        igPushStyleColor(ImGuiCol_ButtonHovered, color_disabled);
                        igPushStyleColor(ImGuiCol_ButtonActive, color_disabled);
                        igPushStyleColor(ImGuiCol_Text, color_black);
                        igButton("load", (struct ImVec2) { 50, 20 });
                        igPopStyleColor(4);
                    }
                    else
                    {
                        sprintf(button_name, "load##%d", slot);
                        if (igButton(button_name, (struct ImVec2) { 50, 20 }))
                            snapshot_load(r->genesis, slot);
                    }
                }
                igEndMenu();
            }
            igSeparator();

            igSliderFloat("Speed", &settings->emulation_speed, 0.2f, 3.0f, "%f", 1.0f);

            if (igMenuItem("Reset", NULL, false, true))
                genesis_initialize(r->genesis);

            if (igMenuItem(r->genesis->status == Status_Running ? "Pause" : "Resume", glfwGetKeyName(GLFW_KEY_P, 0), false, r->genesis->status != Status_NoGameLoaded))
                toggle_pause(r);

            if (igMenuItem("Step", "Space", false, r->genesis->status == Status_Pause))
                step(r);

            igSeparator();
            igMenuItemPtr("Metrics", NULL, &settings->show_metrics, true);
            igMenuItemPtr("Rewinding", NULL, &settings->rewinding_enabled, true);

            igEndMenu();
        }

        if (igBeginMenu("CPU", true))
        {
            igMenuItemPtr("Registers", NULL, &settings->show_m68k_registers, true);
            igMenuItemPtr("Disassembly", NULL, &settings->show_m68k_disassembly, true);
            igMenuItemPtr("Log", NULL, &settings->show_m68k_log, true);
            igSeparator();
            igMenuItemPtr("ROM", NULL, &settings->show_rom, true);
            igMenuItemPtr("RAM", NULL, &settings->show_ram, true);
            igEndMenu();
        }

        if (igBeginMenu("Video", true))
        {

            igSliderFloat("Scaling", &settings->video_scale, 1.0f, 5.0f, "%f", 1.0f);
            if (igMenuItem("Full screen", NULL, settings->full_screen, true))
                toggle_full_screen(r);
            if (igMenuItem("VSync", NULL, settings->vsync, true)) {
                toggle_vsync(r);
            }
            igSeparator();
            igMenuItemPtr("Registers", NULL, &settings->show_vdp_registers, true);
            igMenuItemPtr("Palettes", NULL, &settings->show_vdp_palettes, true);
            igMenuItemPtr("Patterns", NULL, &settings->show_vdp_patterns, true);
            igMenuItemPtr("Planes", NULL, &settings->show_vdp_planes, true);
            igMenuItemPtr("Sprites", NULL, &settings->show_vdp_sprites, true);
            igSeparator();
            igMenuItemPtr("VRAM", NULL, &settings->show_vram, true);
            igMenuItemPtr("VSRAM", NULL, &settings->show_vsram, true);
            igMenuItemPtr("CRAM", NULL, &settings->show_cram, true);
            igEndMenu();
        }

        if (igBeginMenu("Audio", true))
        {
            igMenuItemPtr("Z80 registers", NULL, &settings->show_z80_registers, true);
            igMenuItemPtr("Z80 disassembly", NULL, &settings->show_z80_disassembly, true);
            igMenuItemPtr("Z80 log", NULL, &settings->show_z80_log, true);
            igSeparator();
            igMenuItemPtr("PSG registers", NULL, &settings->show_psg_registers, true);
            igMenuItemPtr("YM2612 registers", NULL, &settings->show_ym2612_registers, true);
            igEndMenu();
        }

        igEndMainMenuBar();
    }

    // M68K registers
    if (settings->show_m68k_registers)
    {
        igBegin("CPU registers", &settings->show_m68k_registers, 0);
        igColumns(2, NULL, true);

        for (int i = 0; i < 8; ++i)
            igText("D%d: %08X", i, r->genesis->m68k->data_registers[i]);

        igNextColumn();

        for (int i = 0; i < 8; ++i)
            igText("A%d: %08X", i, r->genesis->m68k->address_registers[i]);

        igColumns(1, NULL, false);
        igSeparator();

        igText("Status: %04X", r->genesis->m68k->status);

#define STATUS_BIT(label, bit) igText(label); igCheckbox("", & bit); igNextColumn()

        bool extended = EXTENDED(r->genesis->m68k);
        bool negative = NEGATIVE(r->genesis->m68k);
        bool zero = ZERO(r->genesis->m68k);
        bool overflow = OVERFLOW(r->genesis->m68k);
        bool carry = CARRY(r->genesis->m68k);

        igPushStyleColor(ImGuiCol_CheckMark, color_accent);
        igColumns(5, NULL, false);
        STATUS_BIT("X", extended);
        STATUS_BIT("N", negative);
        STATUS_BIT("Z", zero);
        STATUS_BIT("V", overflow);
        STATUS_BIT("C", carry);
        igPopStyleColor(1);

        igColumns(1, NULL, false);
        igSeparator();

        igText("PC:     %08X", r->genesis->m68k->pc);
        igText("Cycles: %08X", r->genesis->m68k->cycles);

        igEnd();
    }

    // M68K disassembly
    if (settings->show_m68k_disassembly)
    {
        igBegin("CPU Disassembly", &settings->show_m68k_disassembly, 0);

        igColumns(4, NULL, false);
        igSetColumnOffset(1, 20);
        igSetColumnOffset(2, 100);

        igNextColumn();
        igText("Address");
        igNextColumn();
        igText("Instruction");
        igNextColumn();
        igText("Opcode");
        igNextColumn();
        igSeparator();

        uint32_t address = r->genesis->m68k->pc;
        for (int i = 0; i < DISASSEMBLY_LENGTH; ++i)
        {
            DecodedInstruction* instr = m68k_decode(r->genesis->m68k, address);

            // The memory may not contain valid opcodes, especially after branching instructions
            if (instr == NULL)
            {
                igColumns(1, NULL, false);
                igTextColored(color_dimmed, "Cannot decode opcode at %06X", address);

                // Fill the disassembler with empty lines to keep the same height
                for (int j = i + 1; j < DISASSEMBLY_LENGTH; ++j)
                    igText("");

                break;
            }

            // Draw a bubble on lines with a breakpoint
            if (debugger_get_breakpoint(r->genesis->debugger, address) != NULL)
            {
                struct ImDrawList* draw_list = igGetWindowDrawList();

                struct ImVec2 window, cursor, bubble;
                igGetWindowPos(&window);
                igGetCursorPos(&cursor);
                bubble = (struct ImVec2) { window.x + cursor.x + 5, window.y + cursor.y + 7 };

                ImDrawList_AddCircleFilled(draw_list, bubble, 4, igGetColorU32Vec(&color_accent), 32);
            }
            igNextColumn();

            igTextColored(i == 0 ? color_accent : color_white, "%06X", address);
            igNextColumn();

            igTextColored(i == 0 ? color_accent : color_white, instr->mnemonics);

            // Toggle breakpoint when the instruction is clicked
            // TODO would be nice to have a hover feedback
            // TODO would be better to click the whole row but grouping seems to be interrupted by columns :(
            if (igIsItemClicked(0))
                debugger_toggle_breakpoint(r->genesis->debugger, address);

            igNextColumn();

            for (int byte = 0; byte < instr->length; ++byte)
            {
                igTextColored(color_dimmed, "%02X ", m68k_read_b(r->genesis->m68k, address + byte));
                igSameLine(0, 0);
            }
            igNextColumn();

            address += instr->length;
            decoded_instruction_free(instr);
        }

        igColumns(1, NULL, false);
        igSeparator();

        igText("Breakpoints");

        for (uint8_t i = 0; i < BREAKPOINTS_COUNT; ++i)
        {
            Breakpoint* b = &r->genesis->debugger->breakpoints[i];

            char name_buffer[100];

            sprintf(name_buffer, "##be%d", i);
            igCheckbox(name_buffer, &b->enabled);

            igSameLine(0, 10);

            sprintf(name_buffer, "##ba%d", i);
            igInputInt(name_buffer, (int*)&b->address, 1, 2, ImGuiInputTextFlags_CharsHexadecimal);
        }

        igEnd();
    }

    // Z80 registers
    if (settings->show_z80_registers)
    {
        Z80* z = r->genesis->z80;

        igBegin("Z80 registers", &settings->show_z80_registers, 0);
        igColumns(2, NULL, true);

        igText("AF: %04X", z->af);
        igText("BC: %04X", z->bc);
        igText("DE: %04X", z->de);
        igText("HL: %04X", z->hl);
        igText("IX: %04X", z->ix);
        igText("I:    %02X", z->i);
        igText("R:    %02X", z->r);

        igNextColumn();

        igText("AF': %04X", z->af_);
        igText("BC': %04X", z->bc_);
        igText("DE': %04X", z->de_);
        igText("HL': %04X", z->hl_);
        igText("IY:  %04X", z->iy);
        igText("PC:  %04X", r->genesis->z80->pc);
        igText("SP:  %04X", r->genesis->z80->sp);

        igColumns(1, NULL, false);
        igSeparator();

        igPushStyleColor(ImGuiCol_CheckMark, color_accent);
        igColumns(2, NULL, false);
        igCheckbox("running", &z->running);
        igNextColumn();
        igCheckbox("resetting", &z->resetting);
        igNextColumn();

        bool sf = z->sf;
        bool zf = z->zf;
        bool yf = z->yf;
        bool hf = z->hf;
        bool xf = z->xf;
        bool pf = z->pf;
        bool nf = z->nf;
        bool cf = z->cf;

        igColumns(8, NULL, false);
        STATUS_BIT("S", sf);
        STATUS_BIT("Z", zf);
        STATUS_BIT("Y", yf);
        STATUS_BIT("H", hf);
        STATUS_BIT("X", xf);
        STATUS_BIT("P", pf);
        STATUS_BIT("N", nf);
        STATUS_BIT("C", cf);
        igPopStyleColor(1);

        igEnd();
    }

    // M68K program log
    if (settings->show_m68k_log)
    {
        igBegin("CPU log", &settings->show_m68k_log, 0);

        Debugger* d = r->genesis->debugger;
        for (int i = 0; i < M68K_LOG_LENGTH; ++i)
        {
            uint32_t address = d->m68k_log_addresses[(d->m68k_log_cursor + 1 + i) % M68K_LOG_LENGTH];
            DecodedInstruction* instr = m68k_decode(r->genesis->m68k, address);
            if (instr != NULL)
            {
                igText("%04X   %s", address, instr->mnemonics);
                decoded_instruction_free(instr);
            }
        }

        igSetScrollHere(0);     // scroll to bottom
        //igButton("clear", )
        igEnd();
    }

    // Z80 disassembly
    if (settings->show_z80_disassembly)
    {
        igBegin("Z80 Disassembly", &settings->show_z80_disassembly, 0);

        igColumns(4, NULL, false);
        igSetColumnOffset(1, 0);
        igSetColumnOffset(2, 60);
        igSetColumnOffset(3, 250);

        igNextColumn();
        igText("Address");
        igNextColumn();
        igText("Instruction");
        igNextColumn();
        igText("Opcode");
        igNextColumn();
        igSeparator();

        uint32_t address = r->genesis->z80->pc;
        for (int i = 0; i < DISASSEMBLY_LENGTH; ++i)
        {
            FullyDecodedZ80Instruction* instr = z80_decode(r->genesis->z80, address);

            // The memory may not contain valid opcodes, especially after branching instructions
            if (instr == NULL)
            {
                igColumns(1, NULL, false);
                igTextColored(color_dimmed, "Cannot decode opcode at %04X", address);

                // Fill the disassembler with empty lines to keep the same height
                for (int j = i + 1; j < DISASSEMBLY_LENGTH; ++j)
                    igText("");

                break;
            }
            igNextColumn();

            igTextColored(i == 0 ? color_accent : color_white, "%04X", address);
            igNextColumn();

            igTextColored(i == 0 ? color_accent : color_white, instr->mnemonics);
            igNextColumn();

            for (int byte = 0; byte < instr->length; ++byte)
            {
                igTextColored(color_dimmed, "%02X ", z80_read(r->genesis->z80, address + byte));
                igSameLine(0, 0);
            }
            igNextColumn();

            address += instr->length;

            fully_decoded_z80_instruction_free(instr);
        }

        igEnd();
    }

    // Z80 program log
    if (settings->show_z80_log)
    {
        igBegin("Z80 log", &settings->show_z80_log, 0);

        Debugger* d = r->genesis->debugger;
        for (int i = 0; i < Z80_LOG_LENGTH; ++i)
        {
            LoggedZ80Instruction* instr = &d->z80_log_instrs[(d->z80_log_cursor + 1 + i) % Z80_LOG_LENGTH];
            igText("%04X   %s", instr->address, instr->mnemonics);
        }

        igSetScrollHere(0);     // scroll to bottom
        igEnd();
    }

    // ROM
    if (settings->show_rom)
        memory_viewer("ROM", &settings->show_rom, r->genesis->rom, Byte, 0x400000, &r->rom_target_address);

    // RAM
    if (settings->show_ram)
        memory_viewer("RAM", &settings->show_ram, r->genesis->ram, Byte, 0x10000, &r->ram_target_address);

    // VDP registers
    if (settings->show_vdp_registers)
    {
        igBegin("VDP registers", &settings->show_vdp_registers, 0);
        igColumns(3, NULL, false);

        Vdp* v = r->genesis->vdp;

#define REGISTER_SECTION(reg) igTextColored(color_title, "Register %0X [%02X]", reg, v->register_raw_values[reg])

        igBeginChild("column1", vec_zero, false, 0);

        REGISTER_SECTION(0);
        igBullet(); text_on_off(v->hblank_interrupt_enabled, "H-blank interrupt:");
        igBullet(); text_on_off(v->hv_counter_latched, "HV-counter latched:");
        igSeparator();

        REGISTER_SECTION(1);
        igBullet(); text_on_off(v->display_enabled, "Display enabled:");
        igBullet(); text_on_off(v->vblank_interrupt_enabled, "V-blank interrupt:");
        igBullet(); text_on_off(v->dma_enabled, "DMA enabled:");
        igBullet(); igText("Display height: %0X", v->display_height);
        igSeparator();

        REGISTER_SECTION(2);
        igBullet(); igText("Plane A location: %0X", v->plane_a_nametable);
        igSeparator();

        REGISTER_SECTION(3);
        igBullet(); igText("Window location: %0X", v->window_nametable);
        igSeparator();

        REGISTER_SECTION(4);
        igBullet(); igText("Plane B location: %0X", v->plane_b_nametable);
        igSeparator();

        REGISTER_SECTION(5);
        igBullet(); igText("Sprite table location: %0X", v->sprites_attribute_table);

        igEndChild();
        igNextColumn();
        igBeginChild("column2", vec_zero, false, 0);

        REGISTER_SECTION(7);
        igBullet(); igText("Background palette: %0X", v->background_color_palette);
        igBullet(); igText("Background color: %0X", v->background_color_entry);
        igSeparator();

        REGISTER_SECTION(0xA);
        igBullet(); igText("H-blank counter: %0X", v->hblank_line);
        igSeparator();

        REGISTER_SECTION(0xB);
        char* vertical_scrolling_mode_names[] = { "Screen", "two columns" };
        igBullet(); igText("Vertical scrolling: %s", vertical_scrolling_mode_names[v->vertical_scrolling_mode]);
        char* horizontal_scrolling_mode_names[] = { "Screen", "Invalid", "Row", "Line" };
        igBullet(); igText("Horizontal scrolling: %s", horizontal_scrolling_mode_names[v->horizontal_scrolling_mode]);

        REGISTER_SECTION(0xC);
        igBullet(); igText("Display width: %0X", v->display_width);
        igBullet(); text_on_off(v->shadow_highlight_enabled, "Shadow/Highlight:");
        igBullet(); igText("Interlace mode: %0X", v->interlace_mode);
        igSeparator();

        REGISTER_SECTION(0xD);
        igBullet(); igText("Horizontal scroll table: %0X", v->horizontal_scrolltable);
        igSeparator();

        REGISTER_SECTION(0xF);
        igBullet(); igText("Auto-increment: %0X", v->auto_increment);

        igEndChild();
        igNextColumn();
        igBeginChild("column3", vec_zero, false, 0);

        REGISTER_SECTION(0x10);
        igBullet(); igText("Plane height: %0X", v->plane_width);
        igBullet(); igText("Plane width: %0X", v->plane_height);
        igSeparator();

        REGISTER_SECTION(0x11);
        igBullet(); igText("Window horizontal direction: %s", v->window_plane_horizontal_direction ? "left" : "right");
        igBullet(); igText("Window horizontal position: %0X", v->window_plane_horizontal_offset);
        igSeparator();

        REGISTER_SECTION(0x12);
        igBullet(); igText("Window vertical direction: %s", v->window_plane_vertical_direction ? "up" : "down");
        igBullet(); igText("Window vertical position: %0X", v->window_plane_vertical_offset);
        igSeparator();

        REGISTER_SECTION(0x13);
        REGISTER_SECTION(0x14);
        igBullet(); igText("DMA length: %0X", v->dma_length);
        igSeparator();

        REGISTER_SECTION(0x15);
        REGISTER_SECTION(0x16);
        REGISTER_SECTION(0x17);
        igBullet(); igText("DMA source: %0X", v->dma_source_address_hi << 16 | v->dma_source_address_lo);

        igEndChild();
        igColumns(1, NULL, false);
        igEnd();
    }

    // VDP palettes
    if (settings->show_vdp_palettes)
    {
        igSetNextWindowSize((struct ImVec2) { 16 * PALETTE_ENTRY_WIDTH, 5 * PALETTE_ENTRY_WIDTH }, 0); // TODO it seems that the title bar is counted in the height...
        igPushStyleVarVec(ImGuiStyleVar_WindowPadding, vec_zero);
        igBegin("VDP palettes", &settings->show_vdp_palettes, ImGuiWindowFlags_NoResize);

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
                ImDrawList_AddRectFilled(draw_list, cell_top_left, cell_bottom_right, 255u << 24 | color.b << 16 | color.g << 8 | color.r, 0, 0);

            }

        igEnd();
        igPopStyleVar(ImGuiStyleVar_WindowPadding);
    }

    // VDP patterns
    if (settings->show_vdp_patterns)
    {
        uint16_t patterns_width = PATTERNS_COLUMNS * 8;
        uint16_t patterns_height = PATTERNS_COUNT / PATTERNS_COLUMNS * 8;

        igPushStyleVarVec(ImGuiStyleVar_WindowPadding, vec_zero);
        igBegin("VDP patterns", &settings->show_vdp_patterns, ImGuiWindowFlags_NoResize);

        // Update the pattern texture with the VRAM contents

        uint8_t patterns_buffer[PATTERNS_COUNT * 64 * 3]; // n patterns * 64 pixels * 3 color components

        for (int pattern = 0; pattern < PATTERNS_COUNT; ++pattern)
        {
            int x = pattern % PATTERNS_COLUMNS * 8;
            int y = pattern / PATTERNS_COLUMNS * 8;
            vdp_draw_pattern(r->genesis->vdp, pattern, debug_palette, patterns_buffer, patterns_width, x, y, false, false);
        }

        glBindTexture(GL_TEXTURE_2D, r->ui_patterns_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, patterns_width, patterns_height, 0, GL_RGB, GL_UNSIGNED_BYTE, patterns_buffer);

        igImage((ImTextureID)(intptr_t)r->ui_patterns_texture, (struct ImVec2) { patterns_width, patterns_height }, vec_zero, vec_one, color_white, color_black);

        // If a pattern is hovered, show a tooltip with a magnified view
        if (igIsItemHovered())
        {
            struct ImVec2 pattern_pos = get_mouse_wrt_window();
            uint16_t pattern_index = (int)pattern_pos.y / 8 * PATTERNS_COLUMNS + (int)pattern_pos.x / 8;

            uint8_t magnified_pattern_buffer[64 * 3];
            vdp_draw_pattern(r->genesis->vdp, pattern_index, debug_palette, magnified_pattern_buffer, 8, 0, 0, false, false);

            glBindTexture(GL_TEXTURE_2D, r->ui_magnified_pattern_texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 8, 8, 0, GL_RGB, GL_UNSIGNED_BYTE, magnified_pattern_buffer);

            igBeginTooltip();
            igText("Pattern #%d", pattern_index);
            igImage((ImTextureID)(intptr_t)r->ui_magnified_pattern_texture, (struct ImVec2) { 8 * PATTERN_MAGNIFICATION_FACTOR, 8 * PATTERN_MAGNIFICATION_FACTOR }, vec_zero, vec_one, color_white, color_white);
            igEndTooltip();
        }

        igEnd();
        igPopStyleVar(ImGuiStyleVar_WindowPadding);
    }

    // VDP planes
    if (settings->show_vdp_planes)
    {
        uint16_t plane_width = 64 * 8;
        uint16_t plane_height = 64 * 8;

        igPushStyleVarVec(ImGuiStyleVar_WindowPadding, vec_zero);
        igBegin("VDP planes", &settings->show_vdp_planes, ImGuiWindowFlags_NoResize);

        // Update the plane texture with the selected plane

        memset(r->plane_buffer, 0, 64 * 8 * 64 * 8 * 3 * sizeof(uint8_t));
        vdp_draw_plane(r->genesis->vdp, r->selected_plane, r->plane_buffer, 512);

        glBindTexture(GL_TEXTURE_2D, r->ui_planes_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, plane_width, plane_height, 0, GL_RGB, GL_UNSIGNED_BYTE, r->plane_buffer);

        igImage((ImTextureID)(intptr_t)r->ui_planes_texture, (struct ImVec2) { plane_width, plane_height }, vec_zero, vec_one, color_white, color_black);

        // If a cell is hovered, show a tooltip with details
        if (igIsItemHovered())
        {
            // Get the data of the hovered cell

            struct ImVec2 cell_pos = get_mouse_wrt_window();
            uint16_t cell_index = (int)cell_pos.y / 8 * r->genesis->vdp->plane_height + (int)cell_pos.x / 8;

            uint16_t pattern_index, palette_index;
            bool priority, horizontal_flip, vertical_flip;
            vdp_get_plane_cell_data(r->genesis->vdp, r->selected_plane, cell_index, &pattern_index, &palette_index, &priority, &horizontal_flip, &vertical_flip);
            
            // Draw a border around the hovered cell

            struct ImVec2 window = get_cursor();
            struct ImVec2 top_left = { window.x + (int)(cell_pos.x / 8) * 8 + 1, window.y + (int)(cell_pos.y / 8) * 8 + 1};
            struct ImVec2 bottom_right = { top_left.x + 8, top_left.y + 8 };

            struct ImDrawList* draw_list = igGetWindowDrawList();
            ImDrawList_AddRect(draw_list, top_left, bottom_right, igGetColorU32Vec(&color_accent), 0, 0, 1);

            // Draw a magnified version of the pattern

            Color* palette = r->genesis->vdp->cram + palette_index * 16;

            uint8_t magnified_pattern_buffer[64 * 3];
            vdp_draw_pattern(r->genesis->vdp, pattern_index, palette, magnified_pattern_buffer, 8, 0, 0, horizontal_flip, vertical_flip);

            glBindTexture(GL_TEXTURE_2D, r->ui_magnified_pattern_texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 8, 8, 0, GL_RGB, GL_UNSIGNED_BYTE, magnified_pattern_buffer);

            igBeginTooltip();
            igText("Cell (%d, %d)", (int)cell_pos.x / 8, (int)cell_pos.y / 8);
            igText("Pattern #%d", pattern_index);

            igText("%-18s", "Horizontal flip: ");
            igSameLine(0, 0);
            igTextColored(horizontal_flip ? color_accent : color_dimmed, horizontal_flip ? "on" : "off");

            igText("%-18s", "Vertical flip: ");
            igSameLine(0, 0);
            igTextColored(vertical_flip ? color_accent : color_dimmed, vertical_flip ? "on" : "off");

            igText("%-18s", "Priority: ");
            igSameLine(0, 0);
            igTextColored(priority ? color_accent : color_dimmed, priority ? "on" : "off");

            igImage((ImTextureID)(intptr_t)r->ui_magnified_pattern_texture, (struct ImVec2) { 8 * PATTERN_MAGNIFICATION_FACTOR, 8 * PATTERN_MAGNIFICATION_FACTOR }, vec_zero, vec_one, color_white, color_white);
            igEndTooltip();
        }

        igColumns(3, NULL, false);
        igRadioButton("Plane A", (int*)&r->selected_plane, Plane_A);
        igNextColumn();
        igRadioButton("Plane B", (int*)&r->selected_plane, Plane_B);
        igNextColumn();
        igRadioButton("Window", (int*)&r->selected_plane, Plane_Window); igColumns(1, NULL, false);

        igEnd();
        igPopStyleVar(ImGuiStyleVar_WindowPadding);
    }

    // VDP sprites
    if (settings->show_vdp_sprites)
    {
        igPushStyleVarVec(ImGuiStyleVar_WindowPadding, vec_zero);
        igBegin("VDP sprites", &settings->show_vdp_sprites, ImGuiWindowFlags_NoResize);

        struct ImVec2 pos = get_cursor();

        // Update the sprite texture

        memset(r->sprites_buffer, 0, 552 * 552 * 3 * sizeof(uint8_t));
        vdp_draw_sprites(r->genesis->vdp, r->sprites_buffer, 552);

        glBindTexture(GL_TEXTURE_2D, r->ui_sprites_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 552, 552, 0, GL_RGB, GL_UNSIGNED_BYTE, r->sprites_buffer);

        igImage((ImTextureID)(intptr_t)r->ui_sprites_texture, (struct ImVec2) { 552, 552 }, vec_zero, vec_one, color_white, color_black);

        // Draw the screen border

        struct ImVec2 a = { pos.x + 128, pos.y + 128 };
        struct ImVec2 b = { a.x + r->genesis->vdp->display_width * 8, a.y + r->genesis->vdp->display_height * 8 };

        struct ImDrawList* draw_list = igGetWindowDrawList();
        ImDrawList_AddRect(draw_list, a, b, igGetColorU32Vec(&color_accent), 0, 0, 1);

        igEnd();
        igPopStyleVar(ImGuiStyleVar_WindowPadding);
    }

    // VRAM
    if (settings->show_vram)
        memory_viewer("VRAM", &settings->show_vram, r->genesis->vdp->vram, Byte, 0x10000, NULL);

    // VSRAM
    if (settings->show_vsram)
        memory_viewer("VSRAM", &settings->show_vsram, r->genesis->vdp->vsram, Word, 0x40, NULL);

    // CRAM
    if (settings->show_cram)
    {
        // Because we do not store raw CRAM data in the VDP implementation, convert
        // the decoded colors back to words for the debug view
        uint16_t raw_cram[0x40];
        for (int c = 0; c < 0x40; ++c)
            raw_cram[c] = COLOR_STRUCT_TO_11(r->genesis->vdp->cram[c]);

        memory_viewer("CRAM", &settings->show_cram, raw_cram, Word, 0x40, NULL);
    }

    // Status bubble when the emulation is paused/rewinding
    // TODO would be better to always keep this overlay on top of the other windows but I don't think it's currently possible
    if (r->genesis->status == Status_Pause || r->genesis->status == Status_Rewinding)
    {
        igSetNextWindowPos((struct ImVec2) { 10, 50 }, 0);
        igPushStyleColor(ImGuiCol_WindowBg, (struct ImVec4) { 1.0f, 1.0f, 1.0f, 0.10f });
        igBegin("Example: Fixed Overlay", &dummy_flag, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
        igTextColored(color_accent, r->genesis->status == Status_Pause ? "Paused" : "Rewinding");
        igEnd();
        igPopStyleColor(1);
    }

    double now = glfwGetTime();
    double dt = now - r->last_time;
    r->last_time = now;

    metric_push(r->tpf, dt * 1000);
    metric_push(r->audio_buffer_queue, SDL_GetQueuedAudioSize(r->genesis->audio->device) / sizeof(int16_t));

    r->metrics_refresh_counter += dt;
    if (r->metrics_refresh_counter > 1) {
        r->metrics_refresh_counter = 0;
        metric_avg(r->tpf);
        metric_avg(r->audio_buffer_queue);
    }

    if (settings->show_metrics) {
        igBegin("Metrics", &settings->show_metrics, 0);
        char buf[50];

        // Time per frame, in milliseconds
        snprintf(buf, sizeof(buf), "tpf (ms)\navg: %.2f\nfps: %.2f",
            r->tpf->avg, r->tpf->avg > 0 ? 1000.0 / r->tpf->avg : 0);
        metric_plot(r->tpf, buf);

        // (M68k) instructions per frame, in kilos
        snprintf(buf, sizeof buf, "audio queue (samples)\navg: %.2f", r->audio_buffer_queue->avg);
        metric_plot(r->audio_buffer_queue, buf);

        igTextColored(color_title, "Remaining audio time: ");
        igSameLine(0,0);
        igText("%.6fms", r->genesis->audio->remaining_time * 1000);

        igTextColored(color_title, "Remaining master cycles");
        igText("M68k:   %d", r->genesis->m68k->remaining_master_cycles);
        igText("Z80:    %d", r->genesis->z80->remaining_master_cycles);
        igText("PSG:    %d", r->genesis->psg->remaining_master_cycles);
        igText("YM2612: %d", r->genesis->ym2612->remaining_master_cycles);

        igEnd();
    }

    // PSG registers
    if (settings->show_psg_registers)
    {
        igBegin("PSG registers", &settings->show_psg_registers, 0);

        PSG* p = r->genesis->psg;

        for (int i=0; i < 3; ++i) {
            igTextColored(color_title, "Square %d", i);
            if (p->square[i].volume == 0xF) {
                igSameLine(0,0);
                igTextColored(color_dimmed, " [silent]");
            }
            igText("volume:  %0X", p->square[i].volume);
            igText("tone:    %0X [%.2fHz]", p->square[i].tone, square_tone_in_hertz(&p->square[i]));
            igText("counter: %0X", p->square[i].counter);
            igText("output:  %d", square_output(&p->square[i]));
        }

        igTextColored(color_title, "Noise");
        if (p->noise.volume == 0xF) {
            igSameLine(0,0);
            igTextColored(color_dimmed, " [silent]");
        }
        igText("volume:     %0X", p->noise.volume);
        igText("mode:       %0X [%s]", p->noise.mode, p->noise.mode ? "white" : "periodic");
        igText("shift rate: %0X", p->noise.shift_rate);
        igText("counter:    %0X", p->noise.counter);
        igText("lfsr:       %0X", p->noise.lfsr);
        igText("output:     %d", noise_output(&p->noise));
        igEnd();
    }

    // YM2612 registers
    if (settings->show_ym2612_registers)
    {
        igBegin("YM2612 registers", &settings->show_ym2612_registers, 0);

        YM2612* y = r->genesis->ym2612;

        // Global registers
        igTextColored(color_title, "Global registers");
        igColumns(3, NULL, false);

        igText("LFO enabled:   %s", y->lfo_enabled ? "true" : "false");
        igText("LFO frequency: %0X", y->lfo_frequency_index);

        igNextColumn();

        igText("timer A: %0X", y->timer_a);
        igText("timer B: %0X", y->timer_b);

        igNextColumn();

        igText("DAC enabled: %0X", y->dac_enabled);
        igText("DAC data:    %0X", y->dac_data);

        igSeparator();

        // Channels

        igColumns(7, NULL, false);

        struct ImVec2 dummy = { 0, 0 };  // Don't know what it does, but needed
                                         // as argument to IgSelectable
        igTextColored(color_title, "Channel");
        igSelectable("enabled", false, ImGuiSelectableFlags_SpanAllColumns, dummy);
        igSelectable("freq number", false, ImGuiSelectableFlags_SpanAllColumns, dummy);
        igSelectable("frequency", false, ImGuiSelectableFlags_SpanAllColumns, dummy);
        igSelectable("feedback", false, ImGuiSelectableFlags_SpanAllColumns, dummy);
        igSelectable("algorithm", false, ImGuiSelectableFlags_SpanAllColumns, dummy);
        igSelectable("stereo", false, ImGuiSelectableFlags_SpanAllColumns, dummy);
        igSelectable("amp modulation", false, ImGuiSelectableFlags_SpanAllColumns, dummy);
        igSelectable("freq modulation", false, ImGuiSelectableFlags_SpanAllColumns, dummy);

        igNextColumn();

        for (int i=0; i < 6; ++i) {
            igTextColored(color_title, "%d", i + 1);

            // FIXME: the checkbox is too tall for the line, shifting all the
            // lines in the table after it.
            igPushStyleColor(ImGuiCol_CheckMark, color_accent);
            igSameLine(20,0);
            char name_buffer[10];
            sprintf(name_buffer, "##chan%d", i);
            igCheckbox(name_buffer, &y->channels[i].muted);
            igPopStyleColor(1);

            if ((i+1) % 3 == 0) {
                uint8_t channel_mode = i == 3 ? y->channel3_mode : y->channel6_mode;

                char* mode_string = "normal";
                switch (channel_mode) {
                case 0: mode_string = "normal"; break;
                case 1: mode_string = "special"; break;
                default: mode_string = "illegal"; break;
                }

                igSameLine(0,0);
                igText(" (%s)", mode_string);
            }

            igText("%d", y->channels[i].enabled);
            igText("%d:%d", y->channels[i].frequency.block, y->channels[i].frequency.freq);
            igText("%.2fHz", channel_frequency_in_hertz(&y->channels[i], genesis_master_frequency(r->genesis)));
            igText("%0X", y->channels[i].feedback);
            igText("%0X", y->channels[i].algorithm);
            igText("%s%s", y->channels[i].left_output ? "L" : " ", y->channels[i].right_output ? "R" : " ");
            igText("%0X", y->channels[i].amplitude_modulation_sensitivity);
            igText("%0X", y->channels[i].frequency_modulation_sensitivity);

            igNextColumn();
        }

        igSeparator();

        // Operators
        for (int i=0; i < 6; ++i) {
            igColumns(1, NULL, false);
            if (igTreeNodePtr((void*)(intptr_t)i, "Operators for channel %d", i+1)) {

                igColumns(5, NULL, false);

                igTextColored(color_title, "Operator");
                igSelectable("detune", false, ImGuiSelectableFlags_SpanAllColumns, dummy);
                igSelectable("multiple", false, ImGuiSelectableFlags_SpanAllColumns, dummy);
                igSelectable("total level", false, ImGuiSelectableFlags_SpanAllColumns, dummy);
                igSelectable("attack rate", false, ImGuiSelectableFlags_SpanAllColumns, dummy);
                igSelectable("decay rate", false, ImGuiSelectableFlags_SpanAllColumns, dummy);
                igSelectable("sustain level", false, ImGuiSelectableFlags_SpanAllColumns, dummy);
                igSelectable("sustain rate", false, ImGuiSelectableFlags_SpanAllColumns, dummy);
                igSelectable("release rate", false, ImGuiSelectableFlags_SpanAllColumns, dummy);
                igSelectable("rate scaling", false, ImGuiSelectableFlags_SpanAllColumns, dummy);
                igSelectable("amp modulation", false, ImGuiSelectableFlags_SpanAllColumns, dummy);

                igNextColumn();

                for (int j=0; j < 4; ++j) {
                    igTextColored(color_title, "%d", j + 1);

                    igText("%0X", y->channels[i].operators[j].detune);
                    igText("%0X", y->channels[i].operators[j].multiple);
                    igText("%0X", y->channels[i].operators[j].total_level);
                    igText("%0X", y->channels[i].operators[j].attack_rate);
                    igText("%0X", y->channels[i].operators[j].decay_rate);
                    igText("%0X", y->channels[i].operators[j].sustain_level);
                    igText("%0X", y->channels[i].operators[j].sustain_rate);
                    igText("%0X", y->channels[i].operators[j].release_rate);
                    igText("%0X", y->channels[i].operators[j].rate_scaling);
                    igText("%0X", y->channels[i].operators[j].amplitude_modulation_enabled);

                    igNextColumn();
                }

                igTreePop();
            }
        }

        igEnd();
    }

    // bool a = true;
    // igShowTestWindow(&a);
}

static void render_ui(Renderer* r)
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

    // Update the projection matrix depending on the window size
    int fb_width = (int)(io->DisplaySize.x * io->DisplayFramebufferScale.x);
    int fb_height = (int)(io->DisplaySize.y * io->DisplayFramebufferScale.y);
    glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);

    const GLfloat ortho_projection[4][4] =
    {
        { 2.0f / io->DisplaySize.x, 0.0f,                      0.0f, 0.0f },
        { 0.0f,                     2.0f / -io->DisplaySize.y, 0.0f, 0.0f },
        { 0.0f,                     0.0f,                     -1.0f, 0.0f },
        { -1.0f,                    1.0f,                      0.0f, 1.0f },
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

            glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)command->TextureId);
            glScissor((int)command->ClipRect.x, (int)(fb_height - command->ClipRect.w), (int)(command->ClipRect.z - command->ClipRect.x), (int)(command->ClipRect.w - command->ClipRect.y));
            glDrawElements(GL_TRIANGLES, (GLsizei)command->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, (GLvoid*)idx_buffer_offset);

            idx_buffer_offset += command->ElemCount;
        }
    }

    glDisable(GL_SCISSOR_TEST);
}

// TODO move this to genesis (via callback)
static void handle_inputs(Renderer* r, int key, int action)
{
    switch (action)
    {
    case GLFW_PRESS:
        switch (key)
        {
        case GLFW_KEY_LEFT  : joypad_press(r->genesis->joypad1, Left); break;
        case GLFW_KEY_RIGHT : joypad_press(r->genesis->joypad1, Right); break;
        case GLFW_KEY_UP    : joypad_press(r->genesis->joypad1, Up); break;
        case GLFW_KEY_DOWN  : joypad_press(r->genesis->joypad1, Down); break;
        case GLFW_KEY_ENTER : joypad_press(r->genesis->joypad1, Start); break;
        case GLFW_KEY_Q     : joypad_press(r->genesis->joypad1, ButtonA); break;
        case GLFW_KEY_W     : joypad_press(r->genesis->joypad1, ButtonB); break;
        case GLFW_KEY_E     : joypad_press(r->genesis->joypad1, ButtonC); break;

        case GLFW_KEY_1: joypad_press(r->genesis->joypad2, Left); break;
        case GLFW_KEY_3: joypad_press(r->genesis->joypad2, Right); break;
        case GLFW_KEY_5: joypad_press(r->genesis->joypad2, Up); break;
        case GLFW_KEY_2: joypad_press(r->genesis->joypad2, Down); break;
        case GLFW_KEY_0: joypad_press(r->genesis->joypad2, Start); break;
        case GLFW_KEY_7: joypad_press(r->genesis->joypad2, ButtonA); break;
        case GLFW_KEY_8: joypad_press(r->genesis->joypad2, ButtonB); break;
        case GLFW_KEY_9: joypad_press(r->genesis->joypad2, ButtonC); break;

        case GLFW_KEY_R:
            if (r->genesis->settings->rewinding_enabled && r->genesis->status == Status_Running)
                r->genesis->status = Status_Rewinding;
            break;
        }
        break;
    case GLFW_RELEASE:
        switch (key)
        {
        case GLFW_KEY_LEFT  : joypad_release(r->genesis->joypad1, Left); break;
        case GLFW_KEY_RIGHT : joypad_release(r->genesis->joypad1, Right); break;
        case GLFW_KEY_UP    : joypad_release(r->genesis->joypad1, Up); break;
        case GLFW_KEY_DOWN  : joypad_release(r->genesis->joypad1, Down); break;
        case GLFW_KEY_ENTER : joypad_release(r->genesis->joypad1, Start); break;
        case GLFW_KEY_Q     : joypad_release(r->genesis->joypad1, ButtonA); break;
        case GLFW_KEY_W     : joypad_release(r->genesis->joypad1, ButtonB); break;
        case GLFW_KEY_E     : joypad_release(r->genesis->joypad1, ButtonC); break;

        case GLFW_KEY_1: joypad_release(r->genesis->joypad2, Left); break;
        case GLFW_KEY_3: joypad_release(r->genesis->joypad2, Right); break;
        case GLFW_KEY_5: joypad_release(r->genesis->joypad2, Up); break;
        case GLFW_KEY_2: joypad_release(r->genesis->joypad2, Down); break;
        case GLFW_KEY_0: joypad_release(r->genesis->joypad2, Start); break;
        case GLFW_KEY_7: joypad_release(r->genesis->joypad2, ButtonA); break;
        case GLFW_KEY_8: joypad_release(r->genesis->joypad2, ButtonB); break;
        case GLFW_KEY_9: joypad_release(r->genesis->joypad2, ButtonC); break;

        case GLFW_KEY_F     : toggle_full_screen(r); break;
        case GLFW_KEY_P     : toggle_pause(r); break;
        case GLFW_KEY_SPACE : step(r); break;

        case GLFW_KEY_ESCAPE:
            if (r->genesis->settings->full_screen)
                toggle_full_screen(r);
            else
                r->genesis->status = Status_Quitting;
            break;

        case GLFW_KEY_R:
            if (r->genesis->status == Status_Rewinding)
                r->genesis->status = Status_Running;
            break;
        }
        break;
    }

}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int modifiers)
{
    Renderer* r = (Renderer*)glfwGetWindowUserPointer(window);
    handle_inputs(r, key, action);

    // Pass the event to the UI

    struct ImGuiIO* io = igGetIO();
    if (action == GLFW_PRESS)
        io->KeysDown[key] = true;
    else if (action == GLFW_RELEASE)
        io->KeysDown[key] = false;

    io->KeyCtrl  = io->KeysDown[GLFW_KEY_LEFT_CONTROL] || io->KeysDown[GLFW_KEY_RIGHT_CONTROL];
    io->KeyShift = io->KeysDown[GLFW_KEY_LEFT_SHIFT]   || io->KeysDown[GLFW_KEY_RIGHT_SHIFT];
    io->KeyAlt   = io->KeysDown[GLFW_KEY_LEFT_ALT]     || io->KeysDown[GLFW_KEY_RIGHT_ALT];
    io->KeySuper = io->KeysDown[GLFW_KEY_LEFT_SUPER]   || io->KeysDown[GLFW_KEY_RIGHT_SUPER];
}

static void char_callback(GLFWwindow* window, unsigned int character)
{
    if (character > 0 && character < 0x10000)
        ImGuiIO_AddInputCharacter(character);
}

static void mouse_move_callback(GLFWwindow* window, double x, double y)
{
    igGetIO()->MousePos = (struct ImVec2) { (float)x, (float)y };
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int modifiers)
{
    igGetIO()->MouseDown[button] = action == GLFW_PRESS;
}

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    igGetIO()->MouseWheel += (float)yoffset;
}

static void drop_callback(GLFWwindow* window, int path_count, const char** paths)
{
    Renderer* r = (Renderer*)glfwGetWindowUserPointer(window);
    genesis_load_rom_file(r->genesis, paths[0]);
}

void window_resize_callback(GLFWwindow* window, int width, int height)
{
    Renderer* r = (Renderer*)glfwGetWindowUserPointer(window);
    r->genesis->settings->window_width = width;
    r->genesis->settings->window_height = height;
}

void window_close_callback(GLFWwindow* window)
{
    Renderer* r = (Renderer*)glfwGetWindowUserPointer(window);
    r->genesis->status = Status_Quitting;
}

void error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW error %d: %s\n", error, description);
}

Renderer* renderer_make(Genesis* genesis)
{
    glfwSetErrorCallback(error_callback);

    printf("Initializing GLFW (%s)...\n", glfwGetVersionString());

    int glfw_success = glfwInit();
    if (glfw_success != GLFW_TRUE)
    {
        FATAL("An error occurred while initializing GLFW");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    GLFWwindow* window = glfwCreateWindow(genesis->settings->window_width, genesis->settings->window_height, "Megado", NULL, NULL);
    if (!window)
    {
        FATAL("An error occurred while creating a GLFW window");
    }

    glfwSetKeyCallback(window, key_callback);
    glfwSetCharCallback(window, char_callback);
    glfwSetCursorPosCallback(window, mouse_move_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetDropCallback(window, drop_callback);
    glfwSetWindowSizeCallback(window, window_resize_callback);
    glfwSetWindowCloseCallback(window, window_close_callback);

    glfwMakeContextCurrent(window);

    printf("Initializing GLEW (%d.%d.%d)...\n", GLEW_VERSION, GLEW_VERSION_MAJOR, GLEW_VERSION_MINOR);

    GLenum glew_success = glewInit();
    if (glew_success != GLEW_OK)
    {
        FATAL("An error occurred while initializing GLEW: %s", glewGetErrorString(glew_success));
    }

    char* gl_version;
    gl_version = (char*)glGetString(GL_VERSION);
    printf("OpenGL version: %s\n", gl_version);

    // Set swap interval according to settings
    glfwSwapInterval(genesis->settings->vsync ? 1 : 0);

    Renderer* r = calloc(1, sizeof(Renderer));
    r->genesis = genesis;
    r->window = window;
    r->plane_buffer = calloc(64 * 8 * 64 * 8 * 3, sizeof(uint8_t));
    r->sprites_buffer = calloc(552 * 552 * 3, sizeof(uint8_t));

    // Store a pointer to the renderer in the window so that it can be accessed from callback functions
    glfwSetWindowUserPointer(r->window, r);

    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);

    r->tpf = metric_make(128);
    r->audio_buffer_queue = metric_make(128);

    init_ui_rendering(r);
    init_genesis_rendering(r);

    return r;
}

void renderer_free(Renderer* r)
{
    if (r == NULL)
        return;

    glDeleteProgram(r->game_shader);
    glDeleteProgram(r->ui_shader);

    igShutdown();
    glfwDestroyWindow(r->window);
    glfwTerminate();

    free(r->plane_buffer);
    free(r->sprites_buffer);
    metric_free(r->tpf);
    metric_free(r->audio_buffer_queue);

    for (int i=0; i < SNAPSHOT_SLOTS; ++i) {
        snapshot_metadata_free(r->snapshots[i]);
    }

    free(r);
}

void renderer_render(Renderer* r)
{
    glClear(GL_COLOR_BUFFER_BIT);

    render_genesis(r);
    render_ui(r);

    glfwSwapBuffers(r->window);
    glfwPollEvents();
}
