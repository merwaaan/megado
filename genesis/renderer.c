#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui/cimgui.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <m68k/instruction.h>
#include <m68k/m68k.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "genesis.h"
#include "joypad.h"
#include "renderer.h"
#include "vdp.h"

#define DISASSEMBLY_LENGTH 20
#define MEMORY_VIEWER_COLUMNS 16

#define PALETTE_ENTRY_WIDTH 16
#define PATTERNS_COUNT 2048
#define PATTERNS_COLUMNS 32

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
        printf("Failed to compile shader\n");
        exit(1);
    }

    return shader;
}

static GLuint create_shader_program(const GLchar* vertex_shader_source, const GLchar* fragment_shader_source)
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

static const GLchar* game_vertex_shader_source =
"#version 330\n"
"uniform mat4 projection_matrix;\n"
"in vec3 vertex_position;\n"
"in vec2 vertex_texcoord;\n"
"out vec2 texcoord;\n"
"void main()\n"
"{\n"
"   texcoord = vertex_texcoord;\n"
"   gl_Position = projection_matrix * vec4(vertex_position, 1.0);\n"
"}\n";

static const GLchar* game_fragment_shader_source =
"#version 330\n"
"uniform sampler2D sampler;\n"
"in vec2 texcoord;\n"
"out vec4 color;\n"
"void main()\n"
"{\n"
"   color = vec4(texture(sampler, texcoord).rgb, 1.0);\n"
"}\n";

static void init_genesis_rendering(Renderer* r)
{

    glGenTextures(1, &r->game_texture);
    glBindTexture(GL_TEXTURE_2D, r->game_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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
    glVertexAttribPointer(shader_texcoord_loc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 3 * sizeof(float));
}

static const GLchar* ui_vertex_shader_source =
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

static const GLchar* ui_fragment_shader_source =
"#version 330\n"
"uniform sampler2D Texture;\n"
"in vec2 Frag_UV;\n"
"in vec4 Frag_Color;\n"
"out vec4 Out_Color;\n"
"void main()\n"
"{\n"
"   Out_Color = Frag_Color * texture( Texture, Frag_UV.st);\n"
"}\n";

static void init_ui_rendering(Renderer* r)
{
    struct ImGuiIO* io = igGetIO();

    igPushStyleVar(ImGuiStyleVar_WindowRounding, 0);

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

    // Setup texture for the patterns and planes debug views

    glGenTextures(1, &r->ui_patterns_texture);
    glBindTexture(GL_TEXTURE_2D, r->ui_patterns_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenTextures(1, &r->ui_planes_texture);
    glBindTexture(GL_TEXTURE_2D, r->ui_planes_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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
    glBindTexture(GL_TEXTURE_2D, r->game_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, BUFFER_WIDTH, BUFFER_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, r->genesis->vdp->buffer);

    // The Genesis' video output will be displayed on the following screen-aligned quad

    int display_width, display_height;
    glfwGetWindowSize(r->window, &display_width, &display_height);
    glViewport(0, 0, display_width, display_height);

    float display_center_x = display_width / 2;
    float display_center_y = display_height / 2;

    float quad_extent = 320.0f / 2 * r->game_scale;
    // TODO handle height separately
    // TODO handle various resolutions
    float quad_vertices[] = {
        // Top-right triangle
        // format: x, y, z, u, v
        display_center_x - quad_extent, display_center_y - quad_extent,  0.0f, 0.0f, 0.0f,
        display_center_x + quad_extent, display_center_y - quad_extent,  0.0f, 1.0f, 0.0f,
        display_center_x + quad_extent, display_center_y + quad_extent,  0.0f, 1.0f, 1.0f,

        // Bottom-let triangle
        display_center_x - quad_extent, display_center_y - quad_extent,  0.0f, 0.0f, 0.0f,
        display_center_x + quad_extent, display_center_y + quad_extent,  0.0f, 1.0f, 1.0f,
        display_center_x - quad_extent, display_center_y + quad_extent,  0.0f, 0.0f, 1.0f
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

static struct ImVec4 color_black = { 0.0f, 0.0f, 0.0f, 1.0f };
static struct ImVec4 color_white = { 1.0f, 1.0f, 1.0f, 1.0f };
static struct ImVec4 color_dimmed = { 0.5f, 0.5f, 0.5f, 1.0f };
static struct ImVec4 color_accent = { 1.0f, 0.07f, 0.57f, 1.0f };
static struct ImVec4 color_sucess = { 0.0f, 1.0f, 0.0f, 1.0f };
static struct ImVec4 color_error = { 1.0f, 0.0f, 0.0f, 1.0f };

static void memory_viewer(char* name, bool* opened, void* data, Size data_size, uint32_t data_length, uint32_t* target_address)
{
    //igSetNextWindowSize((struct ImVec2) { 0, 0 }, 0);
    if (igBegin(name, opened, 0))
    {
        igBeginChild("##memory", (const struct ImVec2) { 600, 300 }, false, 0); // TODO approximate sizing, not sure how to cleanly make the child fit

        // We use a list clipper to browse the whole memory 
        // while rendering only its visible section
        struct ImGuiListClipper list_clipper;
        ImGuiListClipper_Begin(&list_clipper, data_length / MEMORY_VIEWER_COLUMNS, igGetTextLineHeight());

        int first_visible_row = ImGuiListClipper_GetDisplayStart(&list_clipper);
        int last_visible_row = ImGuiListClipper_GetDisplayEnd(&list_clipper);

        for (int row = first_visible_row; row < last_visible_row; ++row)
        {
            uint32_t row_address = row * MEMORY_VIEWER_COLUMNS;
            igText("%06X:  ", row_address);

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
            if (igInputInt("##address", target_address, 16, 32, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_EnterReturnsTrue))
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

static void build_ui(Renderer* r)
{
    // TODO wrapp all the igBegin in if, otherwise collapsed windows still render

    bool dummy_flag = false;

    if (igBeginMainMenuBar())
    {
        if (igBeginMenu("Game", true))
        {
            if (igMenuItem("Reset", NULL, false, true))
                genesis_initialize(r->genesis);

            if (igMenuItem(r->genesis->status == Status_Running ? "Pause" : "Resume", "P", false, r->genesis->status != Status_NoGameLoaded))
                r->genesis->status = r->genesis->status == Status_Pause ? Status_Running : Status_Pause;

            if (igMenuItem("Step", "Space", false, r->genesis->status == Status_Pause))
                genesis_step(r->genesis);

            //igSeparator();
            //igMenuItemPtr("Settings", NULL, &dummy_flag, false);
            igEndMenu();
        }

        if (igBeginMenu("CPU", true))
        {
            igMenuItemPtr("Registers", NULL, &r->show_cpu_registers, true);
            igMenuItemPtr("Disassembly", NULL, &r->show_cpu_disassembly, true);
            igSeparator();
            igMenuItemPtr("ROM", NULL, &r->show_rom, true);
            igMenuItemPtr("RAM", NULL, &r->show_ram, true);
            igEndMenu();
        }

        if (igBeginMenu("Video", true))
        {
            igSliderFloat("Scaling", &r->game_scale, 1.0f, 5.0f, "%f", 1.0f);
            igSeparator();
            igMenuItemPtr("Palettes", NULL, &r->show_vdp_palettes, true);
            igMenuItemPtr("Patterns", NULL, &r->show_vdp_patterns, true);
            igMenuItemPtr("Planes", NULL, &r->show_vdp_planes, true);
            igSeparator();
            igMenuItemPtr("VRAM", NULL, &r->show_vram, &r->show_vram);
            igMenuItemPtr("VSRAM", NULL, &r->show_vsram, &r->show_vsram);
            igMenuItemPtr("CRAM", NULL, &r->show_cram, &r->show_cram);
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
        igBegin("CPU registers", &r->show_cpu_registers, ImGuiWindowFlags_NoResize);
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

    // CPU Disassembly
    if (r->show_cpu_disassembly)
    {
        igBegin("CPU Disassembly", &r->show_cpu_disassembly, 0);

        igColumns(3, NULL, false);
        igText("Address");
        igNextColumn();
        igSetColumnOffset(-1, 70);
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

            igTextColored(i == 0 ? color_accent : color_white, "%06X", address);
            igNextColumn();

            igTextColored(i == 0 ? color_accent : color_white, instr->mnemonics);
            igNextColumn();

            for (int byte = 0; byte < instr->length; ++byte)
            {
                igTextColored(color_dimmed, "%02X ", m68k_read_b(r->genesis->m68k, address + byte));
                igSameLine(0, 0);
            }
            igNextColumn();

            address += instr->length;

            free(instr);
        }

        igColumns(1, NULL, false);
        igSeparator();

        igInputInt("Breakpoint", &r->genesis->m68k->breakpoint, 1, 2, ImGuiInputTextFlags_CharsHexadecimal);
        igEnd();
    }

    // ROM
    if (r->show_rom)
        memory_viewer("ROM", &r->show_rom, r->genesis->memory, Byte, 0x100000, &r->rom_target_address);

    // RAM
    if (r->show_ram)
        memory_viewer("RAM", &r->show_ram, r->genesis->memory + 0xFF000, Byte, 0x10000, &r->ram_target_address);

    // VDP palettes
    if (r->show_vdp_palettes)
    {
        igSetNextWindowSize((struct ImVec2) { 16 * PALETTE_ENTRY_WIDTH, 5 * PALETTE_ENTRY_WIDTH }, 0); // TODO it seems that the title bar is counted in the height...
        igPushStyleVarVec(ImGuiStyleVar_WindowPadding, (struct ImVec2) { 0, 0 });
        igBegin("VDP palettes", &r->show_vdp_palettes, ImGuiWindowFlags_NoResize);

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
        igPopStyleVar(ImGuiStyleVar_WindowPadding);
    }

    // VDP patterns
    if (r->show_vdp_patterns)
    {
        uint16_t patterns_width = PATTERNS_COLUMNS * 8;
        uint16_t patterns_height = PATTERNS_COUNT / PATTERNS_COLUMNS * 8;

        igPushStyleVarVec(ImGuiStyleVar_WindowPadding, (struct ImVec2) { 0, 0 });
        igBegin("VDP patterns", &r->show_vdp_patterns, ImGuiWindowFlags_NoResize);

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

        igImage(r->ui_patterns_texture, (struct ImVec2) { patterns_width, patterns_height }, (struct ImVec2) { 0, 0 }, (struct ImVec2) { 1, 1 }, color_white, color_black);

        igEnd();
        igPopStyleVar(ImGuiStyleVar_WindowPadding);
    }

    // VDP planes
    if (r->show_vdp_planes)
    {
        uint16_t plane_width = 64 * 8;
        uint16_t plane_height = 64 * 8;

        igBegin("VDP planes", &r->show_vdp_planes, ImGuiWindowFlags_NoResize);

        igColumns(3, NULL, false);
        igRadioButton("Plane A", (int*)&r->selected_plane, Plane_A);
        igNextColumn();
        igRadioButton("Plane B", (int*)&r->selected_plane, Plane_B);
        igNextColumn();
        igRadioButton("Window", (int*)&r->selected_plane, Plane_Window); igColumns(1, NULL, false);

        // Update the plane texture with the selected plane

        vdp_draw_plane(r->genesis->vdp, r->selected_plane, r->plane_buffer, 512);

        glBindTexture(GL_TEXTURE_2D, r->ui_planes_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, plane_width, plane_height, 0, GL_RGB, GL_UNSIGNED_BYTE, r->plane_buffer);

        igImage(r->ui_planes_texture, (struct ImVec2) { plane_width, plane_height }, (struct ImVec2) { 0, 0 }, (struct ImVec2) { 1, 1 }, color_white, color_white);

        igEnd();
    }

    // VRAM
    if (r->show_vram)
        memory_viewer("VRAM", &r->show_vram, r->genesis->vdp->vram, Byte, 0x10000, NULL);

    // VSRAM
    if (r->show_vsram)
        memory_viewer("VSRAM", &r->show_vsram, r->genesis->vdp->vsram, Word, 0x40, NULL);

    // CRAM
    if (r->show_cram)
    {
        // Because we do not store raw CRAM data in the VDP implementation, convert
        // the decoded colors back to words for the debug view
        uint16_t raw_cram[0x40];
        for (int c = 0; c < 0x40; ++c)
            raw_cram[c] = COLOR_STRUCT_TO_11(r->genesis->vdp->cram[c]);

        memory_viewer("CRAM", &r->show_cram, raw_cram, Word, 0x40, NULL);
    }

    bool a = true;
    igShowTestWindow(&a);
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

            glBindTexture(GL_TEXTURE_2D, (GLuint)command->TextureId);
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

    io->KeyCtrl = io->KeysDown[GLFW_KEY_LEFT_CONTROL] || io->KeysDown[GLFW_KEY_RIGHT_CONTROL];
    io->KeyShift = io->KeysDown[GLFW_KEY_LEFT_SHIFT] || io->KeysDown[GLFW_KEY_RIGHT_SHIFT];
    io->KeyAlt = io->KeysDown[GLFW_KEY_LEFT_ALT] || io->KeysDown[GLFW_KEY_RIGHT_ALT];
    io->KeySuper = io->KeysDown[GLFW_KEY_LEFT_SUPER] || io->KeysDown[GLFW_KEY_RIGHT_SUPER];
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
    genesis_initialize(r->genesis);
}

void window_close_callback(GLFWwindow* window)
{
    Renderer* r = (Renderer*)glfwGetWindowUserPointer(window);
    r->genesis->status = Status_Quitting;
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

    GLFWwindow* window = glfwCreateWindow(1200, 800, "Megado", NULL, NULL);
    if (!window)
    {
        printf("An error occurred while creating a GLEW window");
        exit(1);
    }

    glfwSetKeyCallback(window, key_callback);
    glfwSetCharCallback(window, char_callback);
    glfwSetCursorPosCallback(window, mouse_move_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetDropCallback(window, drop_callback);
    glfwSetWindowCloseCallback(window, window_close_callback);

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
    r->game_scale = 1.0f;
    r->plane_buffer = calloc(64 * 8 * 64 * 8 * 3, sizeof(uint8_t));

    // Store a pointer to the renderer in the window so that it can be accessed from callback functions
    glfwSetWindowUserPointer(r->window, r);

    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);

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

    free(r->plane_buffer);
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
