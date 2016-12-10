cmake_minimum_required(VERSION 3.7.1)

project(genesis)

file(GLOB M68K_SOURCES "m68k/*.c")
add_library(m68k SHARED ${M68K_SOURCES})

#file(GLOB GENESIS_SOURCES "genesis/*.c")
#add_library(genesis SHARED ${GENESIS_SOURCES})
