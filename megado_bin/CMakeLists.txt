add_executable(megado_bin
    app.cpp
    app.h
    audio.cpp
    audio.h
    core.h
    main.cpp
    metric.cpp
    metric.h
    settings.cpp
    settings.h
    snapshot.cpp
    snapshot.h
    ui.cpp
    ui.h
    video.cpp
    video.h
)

find_package(glfw3 CONFIG REQUIRED)
find_package(GLEW CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)

target_link_libraries(megado_bin
    PRIVATE
        megado_core
        imgui::imgui
        glfw
        GLEW::GLEW
        SDL2::SDL2
        nlohmann_json::nlohmann_json
)

# Prevent GLFW from including gl.h, GL functions come from GLEW
target_compile_definitions(megado_bin
    PRIVATE
        GLFW_INCLUDE_NONE
)
