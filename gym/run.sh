#!/bin/sh

# Script to launch GYM player binary with the dynamic libraries set up

OPTIND=1 # Reset getopts (see https://stackoverflow.com/a/14203146 )

ENV='LD_LIBRARY_PATH=../deps/glew/build/lib:../deps/cimgui/cimgui:../deps/sdl2/install/lib'

DEBUG_DIR='build/debug'
RELEASE_DIR='build/release'

# Parse arguments
JOBS=4
RUNNER=
FLAGS=

while getopts "gvf:j:r:" opt; do
    case "$opt" in
        g) FLAGS="$FLAGS -g"
           ;;
        v) FLAGS="$FLAGS -DDEBUG"
           ;;
        f) FLAGS="$FLAGS $OPTARG"
           ;;
        j) JOBS=$OPTARG
           ;;
        r) RUNNER=$OPTARG
           ;;
    esac
done

shift $((OPTIND-1))

# Parse command
case $1 in
    debug)
        BUILD_DIR=$DEBUG_DIR
        FLAGS="-g $FLAGS"
        ;;
    release)
        BUILD_DIR=$RELEASE_DIR
        FLAGS="-O3 -march=native $FLAGS"
        ;;
    clean)
        make BUILD_DIR=$DEBUG_DIR clean
        make BUILD_DIR=$RELEASE_DIR clean
        exit 0
        ;;
    *)
        echo './run.sh [-gv -j NUM -f FLAGS -r RUNNER] debug|release|clean GYM-FILE'
        exit 1
        ;;
esac
shift

make -j $JOBS BUILD_DIR="$BUILD_DIR" USER_FLAGS="$FLAGS" \
    && env $ENV $RUNNER $BUILD_DIR/gym "$@"
