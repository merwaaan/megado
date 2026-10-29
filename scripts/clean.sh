#!/bin/bash

set -e

cd "$(dirname "$0")/.."

rm -rf _build
rm -rf _build_web
rm -rf megado_web/app/dist
rm -rf megado_web/app/src/wasm
