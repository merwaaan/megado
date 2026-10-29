#!/bin/bash

set -e

cd "$(dirname "$0")/.."

cmake --preset web
cmake --build --preset web

cd megado_web/app
npm run build
