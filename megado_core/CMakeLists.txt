add_library(megado_core STATIC
    debugger.c
    debugger.h
    genesis.c
    genesis.h
    joypad.c
    joypad.h
    psg.c
    psg.h
    snapshot.c
    snapshot.h
    utils.h
    vdp.c
    vdp.h
    ym2612.c
    ym2612.h
    z80.c
    z80.h
    z80_disasm_table_gen.h
    z80_ops.h
    m68k/bit_utils.c
    m68k/bit_utils.h
    m68k/conditions.c
    m68k/conditions.h
    m68k/cycles.c
    m68k/cycles.h
    m68k/instruction.c
    m68k/instruction.h
    m68k/instructions_arithmetic.c
    m68k/instructions_bcd.c
    m68k/instructions_bit.c
    m68k/instructions_control.c
    m68k/instructions_exception.c
    m68k/instructions_logic.c
    m68k/instructions_privileged.c
    m68k/instructions_shift.c
    m68k/instructions_transfer.c
    m68k/m68k.c
    m68k/m68k.h
    m68k/map.c
    m68k/map.h
    m68k/operands.c
    m68k/operands.h
)

target_include_directories(megado_core
    PUBLIC
        "${PROJECT_SOURCE_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}"
)

find_package(miniz CONFIG REQUIRED)

target_link_libraries(megado_core
    PRIVATE
        miniz::miniz
)
