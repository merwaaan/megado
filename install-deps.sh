#!/bin/bash

# GLFW
pushd deps/glfw/
mkdir build
pushd build
cmake -DBUILD_SHARED_LIBS=ON ../
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
popd

# json-c
pushd deps/json-c
sh autogen.sh
./configure --prefix=$PWD
make -j2
make install
popd
