#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui/cimgui.h>
#include <GL/glew.h>
#include <float.h>

#include "../megado/utils.h"
#include "sdl_imgui.h"

static double g_Time = 0.0f;
static bool  g_MousePressed[3] = { false, false, false };
static float g_MouseWheel = 0.0f;

static GLuint ui_font_texture;
static GLuint ui_shader;
static GLuint ui_vertex_array_object, ui_vertex_buffer_object, ui_element_buffer_object;
static GLint ui_shader_texture_loc, ui_shader_projection_loc;

static void render_ui_fn();

static GLuint create_shader(GLenum shader_type, const GLchar* source) {
  GLuint shader = glCreateShader(shader_type);
  glShaderSource(shader, 1, &source, 0);
  glCompileShader(shader);

  GLsizei log_length;
  glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
  if (log_length > 0) {
    GLchar* log = calloc(log_length, sizeof(GLchar));

    glGetShaderInfoLog(shader, log_length, NULL, log);
    printf("%s\n", log);

    free(log);
  }

  GLint success = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (success == GL_FALSE) {
    FATAL("Failed to compile shader");
  }

  return shader;
}

static GLuint create_shader_program(const GLchar* vertex_shader_source, const GLchar* fragment_shader_source) {
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
  if (log_length > 0) {
    GLchar* log = calloc(log_length, sizeof(GLchar));

    glGetProgramInfoLog(program, log_length, NULL, log);
    printf("%s\n", log);

    free(log);
  }

  GLint success;
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (success == GL_FALSE) {
    FATAL("Failed to compile shader program");
  }

  return program;
}

void sdl_imgui_init() {
  struct ImGuiIO* io = igGetIO();

  io->KeyMap[ImGuiKey_Tab] = SDLK_TAB;
  io->KeyMap[ImGuiKey_LeftArrow] = SDL_SCANCODE_LEFT;
  io->KeyMap[ImGuiKey_RightArrow] = SDL_SCANCODE_RIGHT;
  io->KeyMap[ImGuiKey_UpArrow] = SDL_SCANCODE_UP;
  io->KeyMap[ImGuiKey_DownArrow] = SDL_SCANCODE_DOWN;
  io->KeyMap[ImGuiKey_PageUp] = SDL_SCANCODE_PAGEUP;
  io->KeyMap[ImGuiKey_PageDown] = SDL_SCANCODE_PAGEDOWN;
  io->KeyMap[ImGuiKey_Home] = SDL_SCANCODE_HOME;
  io->KeyMap[ImGuiKey_End] = SDL_SCANCODE_END;
  io->KeyMap[ImGuiKey_Delete] = SDLK_DELETE;
  io->KeyMap[ImGuiKey_Backspace] = SDLK_BACKSPACE;
  io->KeyMap[ImGuiKey_Enter] = SDLK_RETURN;
  io->KeyMap[ImGuiKey_Escape] = SDLK_ESCAPE;
  io->KeyMap[ImGuiKey_A] = SDLK_a;
  io->KeyMap[ImGuiKey_C] = SDLK_c;
  io->KeyMap[ImGuiKey_V] = SDLK_v;
  io->KeyMap[ImGuiKey_X] = SDLK_x;
  io->KeyMap[ImGuiKey_Y] = SDLK_y;
  io->KeyMap[ImGuiKey_Z] = SDLK_z;
  io->RenderDrawListsFn = render_ui_fn;
}

void sdl_imgui_create_device_objects() {
  struct ImGuiIO* io = igGetIO();

  const GLchar* ui_vertex_shader_source =
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

  const GLchar* ui_fragment_shader_source =
    "#version 120\n"
    "uniform sampler2D Texture;\n"
    "varying vec2 Frag_UV;\n"
    "varying vec4 Frag_Color;\n"
    "void main()\n"
    "{\n"
    "   gl_FragColor = Frag_Color * texture2D( Texture, Frag_UV.st);\n"
    "}\n";

  // Setup UI shaders
  ui_shader = create_shader_program(ui_vertex_shader_source, ui_fragment_shader_source);
  ui_shader_texture_loc = glGetUniformLocation(ui_shader, "Texture");
  ui_shader_projection_loc = glGetUniformLocation(ui_shader, "ProjMtx");

  glGenBuffers(1, &ui_vertex_buffer_object);
  glGenBuffers(1, &ui_element_buffer_object);

  glGenVertexArrays(1, &ui_vertex_array_object);
  glBindVertexArray(ui_vertex_array_object);
  glBindBuffer(GL_ARRAY_BUFFER, ui_vertex_buffer_object);

  GLint shader_position_loc = glGetAttribLocation(ui_shader, "Position");
  glEnableVertexAttribArray(shader_position_loc);
  glVertexAttribPointer(shader_position_loc, 2, GL_FLOAT, GL_FALSE, sizeof(struct ImDrawVert), (GLvoid*)0);

  GLint shader_texcoord_loc = glGetAttribLocation(ui_shader, "UV");
  glEnableVertexAttribArray(shader_texcoord_loc);
  glVertexAttribPointer(shader_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(struct ImDrawVert), (GLvoid*)sizeof(struct ImVec2));

  GLint shader_color_loc = glGetAttribLocation(ui_shader, "Color");
  glEnableVertexAttribArray(shader_color_loc);
  glVertexAttribPointer(shader_color_loc, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(struct ImDrawVert), (GLvoid*)(2 * sizeof(struct ImVec2)));

  // Init imGui font
  int width, height;
  unsigned char* pixels;
  ImFontAtlas_GetTexDataAsRGBA32(io->Fonts, &pixels, &width, &height, NULL);

  glGenTextures(1, &ui_font_texture);
  glBindTexture(GL_TEXTURE_2D, ui_font_texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

  ImFontAtlas_SetTexID(io->Fonts, (ImTextureID)(intptr_t)ui_font_texture);
}

void sdl_imgui_destroy_device_objects() {
  if (ui_vertex_array_object) {
    glDeleteVertexArrays(1, &ui_vertex_array_object);
    ui_vertex_array_object = 0;
  }

  if (ui_vertex_buffer_object) {
     glDeleteBuffers(1, &ui_vertex_buffer_object);
     ui_vertex_buffer_object = 0;
  }

  if (ui_element_buffer_object) {
     glDeleteBuffers(1, &ui_element_buffer_object);
     ui_element_buffer_object = 0;
  }

  if (ui_shader) {
    glDeleteProgram(ui_shader);
    ui_shader = 0;
  }

  if (ui_font_texture) {
    glDeleteTextures(1, &ui_font_texture);
    ImFontAtlas_SetTexID(igGetIO()->Fonts, 0);
    ui_font_texture = 0;
  }
}

void sdl_imgui_destroy() {
  sdl_imgui_destroy_device_objects();
  igShutdown();
}

static void render_ui_fn(struct ImDrawData *draw_data) {
  struct ImGuiIO* io = igGetIO();
  // Update the projection matrix depending on the window size
  int fb_width = (int)(io->DisplaySize.x * io->DisplayFramebufferScale.x);
  int fb_height = (int)(io->DisplaySize.y * io->DisplayFramebufferScale.y);
  if (fb_width == 0 || fb_height == 0) {
    return;
  }
  ImDrawData_ScaleClipRects(draw_data, io->DisplayFramebufferScale);

  // Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled
  glEnable(GL_BLEND);
  glBlendEquation(GL_FUNC_ADD);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_SCISSOR_TEST);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


  glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
  const GLfloat ortho_projection[4][4] =
    {
     { 2.0f / io->DisplaySize.x, 0.0f,                      0.0f, 0.0f },
     { 0.0f,                     2.0f / -io->DisplaySize.y, 0.0f, 0.0f },
     { 0.0f,                     0.0f,                     -1.0f, 0.0f },
     { -1.0f,                    1.0f,                      0.0f, 1.0f },
    };

  glUseProgram(ui_shader);
  glUniform1i(ui_shader_texture_loc, 0);
  glUniformMatrix4fv(ui_shader_projection_loc, 1, GL_FALSE, &ortho_projection[0][0]);
  glBindVertexArray(ui_vertex_array_object);
  glBindSampler(0, 0);

  for (int n = 0; n < draw_data->CmdListsCount; ++n) {
    struct ImDrawList* commands = draw_data->CmdLists[n];
    ImDrawIdx* idx_buffer_offset = 0;

    int command_count = ImDrawList_GetCmdSize(commands);
    int vertex_count = ImDrawList_GetVertexBufferSize(commands);
    int index_count = ImDrawList_GetIndexBufferSize(commands);

    glBindBuffer(GL_ARRAY_BUFFER, ui_vertex_buffer_object);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)vertex_count * sizeof(struct ImDrawVert),
                 (GLvoid*)ImDrawList_GetVertexPtr(commands, 0), GL_STREAM_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ui_element_buffer_object);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)index_count * sizeof(ImDrawIdx),
                 (GLvoid*)ImDrawList_GetIndexPtr(commands, 0), GL_STREAM_DRAW);

    for (int i = 0; i < command_count; ++i) {
      struct ImDrawCmd* command = ImDrawList_GetCmdPtr(commands, i);

      glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)command->TextureId);
      glScissor((int)command->ClipRect.x,
                (int)(fb_height - command->ClipRect.w),
                (int)(command->ClipRect.z - command->ClipRect.x),
                (int)(command->ClipRect.w - command->ClipRect.y));
      glDrawElements(GL_TRIANGLES, (GLsizei)command->ElemCount,
                     sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
                     (GLvoid*)idx_buffer_offset);

      idx_buffer_offset += command->ElemCount;
    }
  }

  glDisable(GL_SCISSOR_TEST);
}

bool sdl_imgui_process_event(SDL_Event *event) {
  struct ImGuiIO *io = igGetIO();
  switch (event->type) {
  case SDL_MOUSEWHEEL: {
    if (event->wheel.y > 0) g_MouseWheel = +1;
    if (event->wheel.y < 0) g_MouseWheel = -1;
    return true;
  }
  case SDL_MOUSEBUTTONDOWN: {
    if (event->button.button == SDL_BUTTON_LEFT)   g_MousePressed[0] = true;
    if (event->button.button == SDL_BUTTON_RIGHT)  g_MousePressed[1] = true;
    if (event->button.button == SDL_BUTTON_MIDDLE) g_MousePressed[2] = true;
    return true;
  }
  case SDL_TEXTINPUT: {
    ImGuiIO_AddInputCharactersUTF8(event->text.text);
    return true;
  }
  case SDL_KEYDOWN:
  case SDL_KEYUP: {
    int key = event->key.keysym.sym & ~SDLK_SCANCODE_MASK;
    io->KeysDown[key] = (event->type == SDL_KEYDOWN);
    io->KeyShift      = ((SDL_GetModState() & KMOD_SHIFT) != 0);
    io->KeyCtrl       = ((SDL_GetModState() & KMOD_CTRL) != 0);
    io->KeyAlt        = ((SDL_GetModState() & KMOD_ALT) != 0);
    io->KeySuper      = ((SDL_GetModState() & KMOD_GUI) != 0);
    return true;
  }
  }
  return false;
}

void sdl_imgui_new_frame(SDL_Window *window) {
  if (!ui_font_texture) {
    sdl_imgui_create_device_objects();
  }

  struct ImGuiIO* io = igGetIO();

  // Setup display size every frame in case of resizing
  {
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    int display_w, display_h;
    SDL_GL_GetDrawableSize(window, &display_w, &display_h);
    io->DisplaySize = (struct ImVec2) { w, h };
    io->DisplayFramebufferScale = (struct ImVec2)
      { w > 0 ? ((float)display_w / w) : 0,
        h > 0 ? ((float)display_h / h) : 0 };
  }

  // Setup time step
  {
    uint32_t time = SDL_GetTicks();
    double current_time = time / 1000.0;
    io->DeltaTime = g_Time > 0.0 ? (float)(current_time - g_Time) : (float)(1.0f / 60.0f);
    g_Time = current_time;
  }

  // Setup inputs
  {
    int mx, my;
    uint32_t mouseMask = SDL_GetMouseState(&mx, &my);
    if (SDL_GetWindowFlags(window) & SDL_WINDOW_MOUSE_FOCUS) {
      io->MousePos = (struct ImVec2) { (float)mx, (float)my };
    } else {
      io->MousePos = (struct ImVec2) { -FLT_MAX, -FLT_MAX };
    }

    // If a mouse press event came, always pass it as "mouse held this frame", so
    // we don't miss click-release events that are shorter than 1 frame.
    io->MouseDown[0] = g_MousePressed[0] || (mouseMask & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
    io->MouseDown[1] = g_MousePressed[1] || (mouseMask & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;
    io->MouseDown[2] = g_MousePressed[2] || (mouseMask & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0;
    g_MousePressed[0] = g_MousePressed[1] = g_MousePressed[2] = false;

    io->MouseWheel = g_MouseWheel;
    g_MouseWheel = 0.0f;

    SDL_ShowCursor(io->MouseDrawCursor ? 0 : 1);
  }

  igNewFrame();
}
