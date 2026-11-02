#pragma once

extern "C"
{
#include "megado_core/debugger.h"
#include "megado_core/genesis.h"
#include "megado_core/joypad.h"
#include "megado_core/m68k/m68k.h"
#include "megado_core/m68k/map.h"
#include "megado_core/psg.h"
#include "megado_core/snapshot.h"
#include "megado_core/vdp.h"
#include "megado_core/ym2612.h"
#include "megado_core/z80.h"
}

#include <string>

inline std::string rom_name(Genesis* g)
{
    char name[49];
    genesis_get_rom_name(g, name);
    return name;
}
