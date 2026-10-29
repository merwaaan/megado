#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui/cimgui.h>
#include <cimgui/cimgui_impl.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils.h"
#include "video.h"

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
    glAttachShader(program, vertex_shader);

    GLuint fragment_shader = create_shader(GL_FRAGMENT_SHADER, fragment_shader_source);
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

static const GLchar* vertex_shader_source =
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

static const GLchar* fragment_shader_source =
    "#version 120\n"
    "uniform sampler2D sampler;\n"
    "varying vec2 texcoord;\n"
    "void main()\n"
    "{\n"
    "   gl_FragColor = vec4(texture2D(sampler, texcoord).rgb, 1.0);\n"
    "}\n";

static void init_rendering(Video* v)
{
    // Texture

    glGenTextures(1, &v->texture);
    glBindTexture(GL_TEXTURE_2D, v->texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Shader

    v->shader = create_shader_program(vertex_shader_source, fragment_shader_source);

    v->shader_projection_loc = glGetUniformLocation(v->shader, "projection_matrix");

    // Buffers

    glGenBuffers(1, &v->vertex_buffer_object);
    glBindBuffer(GL_ARRAY_BUFFER, v->vertex_buffer_object);

    glGenVertexArrays(1, &v->vertex_array_object);
    glBindVertexArray(v->vertex_array_object);

    GLint shader_position_loc = glGetAttribLocation(v->shader, "vertex_position");
    glEnableVertexAttribArray(shader_position_loc);
    glVertexAttribPointer(shader_position_loc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);

    GLint shader_texcoord_loc = glGetAttribLocation(v->shader, "vertex_texcoord");
    glEnableVertexAttribArray(shader_texcoord_loc);
    glVertexAttribPointer(shader_texcoord_loc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)(3 * sizeof(float)));
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int modifiers)
{
    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, modifiers);
    Video* v = (Video*)glfwGetWindowUserPointer(window);
    v->callbacks.key(v->callbacks.app_ptr, key, action);
}

static void char_callback(GLFWwindow* window, unsigned int character)
{
    ImGui_ImplGlfw_CharCallback(window, character);
}

static void mouse_move_callback(GLFWwindow* window, double x, double y)
{
    ImGui_ImplGlfw_CursorPosCallback(window, x, y);
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int modifiers)
{
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, modifiers);
}

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
}

static void drop_callback(GLFWwindow* window, int path_count, const char** paths)
{
    Video* v = (Video*)glfwGetWindowUserPointer(window);
    v->callbacks.drop(v->callbacks.app_ptr, paths[0]);
}

static void window_resize_callback(GLFWwindow* window, int width, int height)
{
    Video* v = (Video*)glfwGetWindowUserPointer(window);
    v->callbacks.resize(v->callbacks.app_ptr, width, height);
}

static void window_close_callback(GLFWwindow* window)
{
    Video* v = (Video*)glfwGetWindowUserPointer(window);
    v->callbacks.close(v->callbacks.app_ptr);
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW error %d: %s\n", error, description);
}

Video* video_make(int width, int height, VideoCallbacks callbacks)
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

    GLFWwindow* window = glfwCreateWindow(width, height, "Megado", NULL, NULL);

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

    glfwSwapInterval(1);

    Video* v = calloc(1, sizeof(Video));
    v->callbacks = callbacks;
    v->window = window;

    // Store a pointer to the video in the window so that it can be accessed from callback functions
    glfwSetWindowUserPointer(v->window, v);

    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);

    igCreateContext(NULL);
    ImGui_ImplGlfw_InitForOpenGL(window, false);
    ImGui_ImplOpenGL2_Init();

    init_rendering(v);

    return v;
}

void video_free(Video* v)
{
    if (v == NULL)
    {
        return;
    }

    glDeleteProgram(v->shader);

    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    igDestroyContext(NULL);

    glfwDestroyWindow(v->window);
    glfwTerminate();

    free(v);
}

void video_set_full_screen(Video* v, bool full_screen)
{
    if (full_screen)
    {
        // Save the current size to restore it later

        glfwGetWindowSize(v->window, &v->window_previous_width, &v->window_previous_height);

        // Switch

        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        glfwSetWindowMonitor(v->window, monitor, 0, 0, mode->width, mode->height, GLFW_DONT_CARE);
    }
    else
    {
        glfwSetWindowMonitor(v->window, NULL, 50, 50, v->window_previous_width, v->window_previous_height, GLFW_DONT_CARE);
    }
}

void video_begin_frame(Video* v)
{
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    igNewFrame();
}

void video_end_frame(Video* v)
{
    igRender();
    ImGui_ImplOpenGL2_RenderDrawData(igGetDrawData());

    glfwSwapBuffers(v->window);
    glfwPollEvents();
}

void video_draw(Video* v, const uint8_t* pixels, uint16_t width, uint16_t height, uint32_t pitch, float scale)
{
    // Store the framebuffer's pixels in the texture

    glBindTexture(GL_TEXTURE_2D, v->texture);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, pitch / 3); // Whatever the current width of the output, be sure to consider the buffer full width (in low resolution mode, the rightmost pixels will be skipped)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

    // Prepare a screen-aligned quad

    int display_width, display_height;
    glfwGetWindowSize(v->window, &display_width, &display_height);
    glViewport(0, 0, display_width, display_height);

    float display_center_x = display_width / 2.0f;
    float display_center_y = display_height / 2.0f;

    float output_half_width = width / 2.0f * scale;
    float output_half_height = height / 2.0f * scale;

    float quad_vertices[] = {
        // Top-right triangle
        // format: x, y, z, u, v

        display_center_x - output_half_width, display_center_y - output_half_height, 0.0f, 0.0f, 0.0f,
        display_center_x + output_half_width, display_center_y - output_half_height, 0.0f, 1.0f, 0.0f,
        display_center_x + output_half_width, display_center_y + output_half_height, 0.0f, 1.0f, 1.0f,

        // Bottom-let triangle

        display_center_x - output_half_width, display_center_y - output_half_height, 0.0f, 0.0f, 0.0f,
        display_center_x + output_half_width, display_center_y + output_half_height, 0.0f, 1.0f, 1.0f,
        display_center_x - output_half_width, display_center_y + output_half_height, 0.0f, 0.0f, 1.0f};

    glBindBuffer(GL_ARRAY_BUFFER, v->vertex_buffer_object);
    glBufferData(GL_ARRAY_BUFFER, 30 * sizeof(float), quad_vertices, GL_DYNAMIC_DRAW);

    // Update the projection matrix depending on the window size

    const GLfloat ortho_projection[4][4] =
        {
            {2.0f / display_width, 0.0f, 0.0f, 0.0f},
            {0.0f, 2.0f / -display_height, 0.0f, 0.0f},
            {0.0f, 0.0f, -1.0f, 0.0f},
            {-1.0f, 1.0f, 0.0f, 1.0f},
        };

    glUseProgram(v->shader);
    glUniformMatrix4fv(v->shader_projection_loc, 1, GL_FALSE, &ortho_projection[0][0]);

    // Draw the quad

    glBindVertexArray(v->vertex_array_object);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDrawArrays(GL_TRIANGLES, 3, 3);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);
}
