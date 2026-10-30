[![Build](https://github.com/merwaaan/megado/actions/workflows/build.yml/badge.svg)](https://github.com/merwaaan/megado/actions/workflows/build.yml)

A Genesis/Megadrive emulator in C, with a C++ desktop frontend and a TypeScript web frontend.

## Build

First, initialize the dependencies.

```
scripts/init.sh
```

### Desktop

```
scripts/build.sh [release]
_build/megado_bin/Debug/megado_bin ROM
```

### Web

```
scripts/build-web.sh
cd megado_web/app
npm run dev
```
