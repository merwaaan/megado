cmake_minimum_required(VERSION 4.2)

project(megado C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_subdirectory(megado_core)
add_subdirectory(megado_web/wasm)

if(NOT EMSCRIPTEN)
    enable_testing()

    add_subdirectory(megado_bin)
    add_subdirectory(megado_test)
endif()
