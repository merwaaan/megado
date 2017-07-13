[![Build Status](https://travis-ci.org/merwaaan/megado.svg?branch=master)](https://travis-ci.org/merwaaan/megado)
[![Build status](https://ci.appveyor.com/api/projects/status/1byv0ynwt69rxgx9?svg=true)](https://ci.appveyor.com/project/merwaaan/megado)

Work in progress.

## Build

### Linux

```
git clone git@github.com:merwaaan/megado.git
git submodule update --init --recursive
./install-deps.sh
./run.sh release ROM
```

`install-deps.sh` builds the dependencies in their own folder, and does not
install anything into `/usr`.

The `./run.sh` script builds and runs the emulator adding the dependencies on
`LD_LIBRARY_PATH` if the build succeeds.

You can also wrap the binary with tools like `valgrind` or `gdb`:

```
./run.sh -r `valgrind --leak-check=full` debug ROM
./run.sh -r `gdb --args` debug ROM
```

In this case, using the `debug` target (no optimizations, debug symbols) is
preferable.

### Windows

First, initialize the dependencies (requires Msys and Python).

```
git submodule update --init --recursive
./install-deps-win.sh
```

Then, use the main MSVC solution `megado.sln`.
