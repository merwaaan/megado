#!/bin/bash

set -e

cd "$(dirname "$0")/.."

PRESET=${1:-debug}

cmake --preset $PRESET
cmake --build --preset $PRESET
