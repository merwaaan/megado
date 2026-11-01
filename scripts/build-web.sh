#!/bin/bash

set -e

cd "$(dirname "$0")/.."

# Emscripten needs tsc to generate the wasm typings
export PATH="$PWD/megado_web/app/node_modules/.bin:$PATH"

cmake --preset web
cmake --build --preset web

cd megado_web/app
npm run build
