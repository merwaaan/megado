#pragma once

#include <GLFW/glfw3.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct VideoCallbacks
{
    void* app_ptr;
    void (*key)(void* app_ptr, int key, int action);
    void (*drop)(void* app_ptr, const char* path);
    void (*resize)(void* app_ptr, int width, int height);
    void (*close)(void* app_ptr);
} VideoCallbacks;

typedef struct Video
{
    VideoCallbacks callbacks;

    GLFWwindow* window;

    // Window dimensions to be restored when leaving full screen
    int window_previous_width, window_previous_height;

    // Rendering resources
    GLuint shader;
    GLuint texture;
    GLuint vertex_array_object, vertex_buffer_object;
    GLint shader_projection_loc;
} Video;

Video* video_make(int width, int height, VideoCallbacks callbacks);
void video_free(Video*);

void video_set_full_screen(Video*, bool full_screen);

void video_begin_frame(Video*);
void video_draw(Video*, const uint8_t* pixels, uint16_t width, uint16_t height, uint32_t pitch, float scale);
void video_end_frame(Video*);
