#!/bin/sh

# Script to launch Megado debug or launch binaries with the dynamic libraries
# from the submodules.  E.g.  `./run.sh release` will make the release binary,
# and if it succeeds, will launch Megado in release mode.
#
# The `-g` flag will run the binary through GDB, passing any extra arguments.

OPTIND=1 # Reset getopts (see https://stackoverflow.com/a/14203146)

ENV='LD_LIBRARY_PATH=deps/cimgui/cimgui:deps/glfw/build/src:deps/glew/build/lib'
DEBUG_BIN='build/debug/megado'
RELEASE_BIN='build/release/megado'

# Parse arguments
JOBS=4
MAYBE_GDB=

while getopts "gj:" opt; do
    case "$opt" in
        g) MAYBE_GDB='gdb --args'
           ;;
        j) JOBS=$OPTARG
           ;;
    esac
done

shift $((OPTIND-1))

# Parse command
case $1 in
    debug) TARGET=debug
           BIN=$DEBUG_BIN
           ;;
    release) TARGET=release
             BIN=$RELEASE_BIN
             ;;
    *)
        echo './run.sh [-g -j NUM] debug|release ROM'
        exit 0
        ;;
esac
shift

make -j $JOBS $BUILD_TARGET && env $ENV $MAYBE_GDB $BIN "$@"
