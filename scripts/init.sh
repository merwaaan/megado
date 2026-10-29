#!/bin/bash

set -e

cd "$(dirname "$0")/.."

# vcpkg

git submodule update --init external/vcpkg external/emsdk

# emscripten

EMSCRIPTEN_VERSION=6.0.13
NINJA_TOOL=ninja-1.13.2-64bit

./external/emsdk/emsdk install $EMSCRIPTEN_VERSION $NINJA_TOOL
./external/emsdk/emsdk activate --embedded $EMSCRIPTEN_VERSION $NINJA_TOOL

# Generate the build files

cmake --preset debug
cmake --preset web

# Install the dependencies for the web app

cd megado_web/app
npm install
