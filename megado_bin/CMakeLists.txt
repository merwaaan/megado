find_package(cimgui CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(GLEW CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(json-c CONFIG REQUIRED)

add_executable(megado_bin
    audio.c
    audio.h
    main.c
    metric.c
    metric.h
    settings.c
    settings.h
    snapshot.c
    snapshot.h
    ui.c
    ui.h
    video.c
    video.h
)

target_link_libraries(megado_bin
    PRIVATE
        megado_core
        cimgui::cimgui
        glfw
        GLEW::GLEW
        SDL2::SDL2
        json-c::json-c
)
