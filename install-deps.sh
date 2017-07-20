#!/bin/bash

# GLFW
pushd deps/glfw/
mkdir build
pushd build
cmake ../
make -j2
popd
popd

# GLEW
# (doesn't behave well with -j2)
pushd deps/glew/
pushd auto
make
popd
pushd build
cmake ./cmake
make
popd
popd

# cimgui
pushd deps/cimgui/cimgui
make -j2
# Build static library since cimgui doesn't do it itself
ar rcs cimgui.a cimgui.o fontAtlas.o drawList.o listClipper.o ../imgui/imgui.o ../imgui/imgui_draw.o ../imgui/imgui_demo.o
popd

# json-c
pushd deps/json-c
sh autogen.sh
./configure --prefix=$PWD
make -j2
make install
popd
