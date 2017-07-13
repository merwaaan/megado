[![Build Status](https://travis-ci.org/merwaaan/megado.svg?branch=master)](https://travis-ci.org/merwaaan/megado)

Work in progress.

## Build

### Linux

```
git clone git@github.com:merwaaan/megado.git
git submodule update --init --recursive
./install-deps.sh
make
```

`install-deps.sh` builds the dependencies in their own folder, and does not
install anything into `/usr`.

The `./run.sh` script builds and runs the emulator adding the dependencies on
`LD_LIBRARY_PATH` if the build succeeds:

```
./run.sh release ROM
```

### Windows

First, initialize the dependencies (requires Msys and Python).

```
git submodule update --init --recursive
./install-deps-win.sh
```

Then, use the main MSVC solution `megado.sln`.
