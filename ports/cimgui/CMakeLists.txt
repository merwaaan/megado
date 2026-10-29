cmake_minimum_required(VERSION 3.15)

project(cimgui CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

add_library(cimgui STATIC
    cimgui.cpp
    cimgui_impl.cpp
    imgui/imgui.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_opengl2.cpp
)

target_compile_definitions(cimgui
    PUBLIC
        CIMGUI_NO_EXPORT
        CIMGUI_USE_GLFW
        CIMGUI_USE_OPENGL2
        IMGUI_DISABLE_OBSOLETE_FUNCTIONS=1
    PRIVATE
        "IMGUI_USER_CONFIG=\"../cimconfig.h\""
        "IMGUI_IMPL_API=extern\t\"C\"\t"
)

target_include_directories(cimgui
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui"
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends"
    INTERFACE
        "$<INSTALL_INTERFACE:include>"
)

target_link_libraries(cimgui PUBLIC glfw OpenGL::GL)

if(WIN32)
    target_link_libraries(cimgui PUBLIC imm32)
endif()

install(TARGETS cimgui EXPORT cimgui-targets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES cimgui.h cimgui_impl.h DESTINATION include/cimgui)

install(EXPORT cimgui-targets NAMESPACE cimgui:: DESTINATION share/cimgui)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/cimgui-config.cmake"
"include(CMakeFindDependencyMacro)
find_dependency(glfw3 CONFIG)
find_dependency(OpenGL)
include(\"\${CMAKE_CURRENT_LIST_DIR}/cimgui-targets.cmake\")
")

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/cimgui-config.cmake" DESTINATION share/cimgui)
