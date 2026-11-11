find_path(GREATEST_INCLUDE_DIR greatest.h REQUIRED)

add_executable(megado_test
    addressing_modes.c
    bit_utils.c
    globals.c
    globals.h
    instructions_arithmetic.c
    instructions_bcd.c
    instructions_bit.c
    instructions_logic.c
    instructions_shift.c
    instructions_transfer.c
    m68k_io.c
    main.c
)

target_include_directories(megado_test PRIVATE "${GREATEST_INCLUDE_DIR}")

target_link_libraries(megado_test PRIVATE megado_core)

add_test(NAME megado_test COMMAND megado_test)
