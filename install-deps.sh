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
pushd deps/glew
pushd auto
make
popd
pushd build
cmake ./cmake
make -j2 glew
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

# SDL2
pushd deps/sdl2
./configure --prefix=$PWD/install --disable-mir-shared --disable-video\
            --disable-render --disable-events --disable-joystick\
            --disable-haptic --disable-power --disable-filesystem\
            --disable-file --disable-threads --disable-timers\
            --disable-libudev --disable-dbus --disable-ime
make -j2
make install
popd
