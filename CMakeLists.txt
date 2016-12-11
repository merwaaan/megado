cmake_minimum_required(VERSION 3.7.1)

project(genesis)

#SET(CMAKE_C_FLAGS "-std=c99")

include_directories(".")

file(GLOB M68K_SOURCES "m68k/*.c")
add_library(m68k STATIC ${M68K_SOURCES})
set_property(TARGET m68k PROPERTY C_STANDARD 99)

file(GLOB GENESIS_SOURCES "genesis/*.c")
add_library(genesis STATIC ${GENESIS_SOURCES})
set_property(TARGET genesis PROPERTY C_STANDARD 99)