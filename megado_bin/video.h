#pragma once

#include <cstdint>
#include <functional>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <span>

struct VideoCallbacks
{
    std::function<void(int key, int action)> key;
    std::function<void(const char* path)> drop;
    std::function<void(int width, int height)> resize;
    std::function<void()> close;
};

class Video
{
public:
    Video(int width, int height, VideoCallbacks callbacks);
    ~Video();

    Video(const Video&) = delete;
    Video& operator=(const Video&) = delete;

    void set_full_screen(bool full_screen);

    void begin_frame();
    void draw(std::span<const uint8_t> pixels, uint16_t width, uint16_t height, uint32_t pitch, float scale);
    void end_frame();

private:
    void init_rendering();

    GLFWwindow* window = nullptr;

    // Window dimensions to be restored when leaving full screen
    int window_previous_width = 0, window_previous_height = 0;

    // Rendering resources
    GLuint shader = 0;
    GLuint texture = 0;
    GLuint vertex_array_object = 0, vertex_buffer_object = 0;
    GLint shader_projection_loc = 0;

    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int modifiers);
    static void drop_callback(GLFWwindow* window, int path_count, const char** paths);
    static void window_resize_callback(GLFWwindow* window, int width, int height);
    static void window_close_callback(GLFWwindow* window);

    VideoCallbacks callbacks;
};
