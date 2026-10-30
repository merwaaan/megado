#include <cstdlib>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl2.h>
#include <print>
#include <string>

#include "video.h"

using namespace std;

[[noreturn]] static void fatal(const string& message)
{
    println(stderr, "FATAL: {}", message);
    exit(1);
}

static GLuint create_shader(GLenum shader_type, const GLchar* source)
{
    GLuint shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLsizei log_length;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);

    if (log_length > 0)
    {
        string log(log_length, '\0');

        glGetShaderInfoLog(shader, log_length, nullptr, log.data());
        println("{}", log);
    }

    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (success == GL_FALSE)
    {
        fatal("Failed to compile shader");
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
        string log(log_length, '\0');

        glGetProgramInfoLog(program, log_length, nullptr, log.data());
        println("{}", log);
    }

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (success == GL_FALSE)
    {
        fatal("Failed to compile shader program");
    }

    return program;
}

static const GLchar* vertex_shader_source = R"(#version 120
uniform mat4 projection_matrix;

attribute vec3 vertex_position;
attribute vec2 vertex_texcoord;

varying vec2 texcoord;

void main()
{
   texcoord = vertex_texcoord;
   gl_Position = projection_matrix * vec4(vertex_position, 1.0);
}
)";

static const GLchar* fragment_shader_source = R"(#version 120
uniform sampler2D sampler;

varying vec2 texcoord;

void main()
{
   gl_FragColor = vec4(texture2D(sampler, texcoord).rgb, 1.0);
}
)";

static Video* video_from_window_data(GLFWwindow* window)
{
    return static_cast<Video*>(glfwGetWindowUserPointer(window));
}

void Video::key_callback(GLFWwindow* window, int key, int scancode, int action, int modifiers)
{
    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, modifiers);
    video_from_window_data(window)->callbacks.key(key, action);
}

void Video::drop_callback(GLFWwindow* window, int path_count, const char** paths)
{
    video_from_window_data(window)->callbacks.drop(paths[0]);
}

void Video::window_resize_callback(GLFWwindow* window, int width, int height)
{
    video_from_window_data(window)->callbacks.resize(width, height);
}

void Video::window_close_callback(GLFWwindow* window)
{
    video_from_window_data(window)->callbacks.close();
}

static void error_callback(int error, const char* description)
{
    println(stderr, "GLFW error {}: {}", error, description);
}

Video::Video(int width, int height, VideoCallbacks callbacks)
    : callbacks(move(callbacks))
{
    glfwSetErrorCallback(error_callback);

    println("Initializing GLFW ({})...", glfwGetVersionString());

    if (glfwInit() != GLFW_TRUE)
    {
        fatal("An error occurred while initializing GLFW");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    window = glfwCreateWindow(width, height, "Megado", nullptr, nullptr);

    if (!window)
    {
        fatal("An error occurred while creating a GLFW window");
    }

    // Store a pointer to the video in the window so that it can be accessed from callback functions
    glfwSetWindowUserPointer(window, this);

    glfwSetKeyCallback(window, key_callback);
    glfwSetCharCallback(window, ImGui_ImplGlfw_CharCallback);
    glfwSetCursorPosCallback(window, ImGui_ImplGlfw_CursorPosCallback);
    glfwSetMouseButtonCallback(window, ImGui_ImplGlfw_MouseButtonCallback);
    glfwSetScrollCallback(window, ImGui_ImplGlfw_ScrollCallback);
    glfwSetDropCallback(window, drop_callback);
    glfwSetWindowSizeCallback(window, window_resize_callback);
    glfwSetWindowCloseCallback(window, window_close_callback);

    glfwMakeContextCurrent(window);

    println("Initializing GLEW ({}.{}.{})...", GLEW_VERSION, GLEW_VERSION_MAJOR, GLEW_VERSION_MINOR);

    GLenum glew_success = glewInit();

    if (glew_success != GLEW_OK)
    {
        fatal(string("An error occurred while initializing GLEW: ") + reinterpret_cast<const char*>(glewGetErrorString(glew_success)));
    }

    println("OpenGL version: {}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));

    glfwSwapInterval(1);

    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);

    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, false);
    ImGui_ImplOpenGL2_Init();

    init_rendering();
}

Video::~Video()
{
    glDeleteProgram(shader);

    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}

void Video::init_rendering()
{
    // Texture

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Shader

    shader = create_shader_program(vertex_shader_source, fragment_shader_source);

    shader_projection_loc = glGetUniformLocation(shader, "projection_matrix");

    // Buffers

    glGenBuffers(1, &vertex_buffer_object);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);

    glGenVertexArrays(1, &vertex_array_object);
    glBindVertexArray(vertex_array_object);

    GLint shader_position_loc = glGetAttribLocation(shader, "vertex_position");
    glEnableVertexAttribArray(shader_position_loc);
    glVertexAttribPointer(shader_position_loc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);

    GLint shader_texcoord_loc = glGetAttribLocation(shader, "vertex_texcoord");
    glEnableVertexAttribArray(shader_texcoord_loc);
    glVertexAttribPointer(shader_texcoord_loc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<const GLvoid*>(3 * sizeof(float)));
}

void Video::set_full_screen(bool full_screen)
{
    if (full_screen)
    {
        // Save the current size to restore it later

        glfwGetWindowSize(window, &window_previous_width, &window_previous_height);

        // Switch

        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, GLFW_DONT_CARE);
    }
    else
    {
        glfwSetWindowMonitor(window, nullptr, 50, 50, window_previous_width, window_previous_height, GLFW_DONT_CARE);
    }
}

void Video::begin_frame()
{
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Video::end_frame()
{
    ImGui::Render();
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
    glfwPollEvents();
}

void Video::draw(span<const uint8_t> pixels, uint16_t width, uint16_t height, uint32_t pitch, float scale)
{
    // Store the framebuffer's pixels in the texture

    glBindTexture(GL_TEXTURE_2D, texture);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, pitch / 3); // Whatever the current width of the output, be sure to consider the buffer full width (in low resolution mode, the rightmost pixels will be skipped)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

    // Prepare a screen-aligned quad

    int display_width, display_height;
    glfwGetWindowSize(window, &display_width, &display_height);
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

    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_DYNAMIC_DRAW);

    // Use a projection matrix that matches the window size

    const GLfloat ortho_projection[4][4] =
        {
            {2.0f / display_width, 0.0f, 0.0f, 0.0f},
            {0.0f, 2.0f / -display_height, 0.0f, 0.0f},
            {0.0f, 0.0f, -1.0f, 0.0f},
            {-1.0f, 1.0f, 0.0f, 1.0f},
        };

    glUseProgram(shader);
    glUniformMatrix4fv(shader_projection_loc, 1, GL_FALSE, &ortho_projection[0][0]);

    // Draw the quad

    glBindVertexArray(vertex_array_object);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDrawArrays(GL_TRIANGLES, 3, 3);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);
}
