#!/bin/bash

# GLFW
pushd deps/glfw/
mkdir build
pushd build
cmake -DBUILD_SHARED_LIBS=ON ../
make -j2
popd
popd

# GLEW (can't build from git checkout, see https://github.com/nigels-com/glew/issues/85 )
pushd deps
wget https://github.com/nigels-com/glew/releases/download/glew-2.1.0/glew-2.1.0.tgz
tar xf glew-2.1.0.tgz
mv glew-2.1.0 glew
pushd glew/build
cmake ./cmake
make -j2
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
