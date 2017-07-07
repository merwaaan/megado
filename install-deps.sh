#!/bin/sh

# GLFW
pushd deps/glfw/
mkdir build
pushd build
cmake -DBUILD_SHARED_LIBS=ON ../
make
popd
popd

# GLEW
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
make
popd

# json-c
pushd deps/json-c
./configure --prefix=$(PWD)
make
make install
popd
