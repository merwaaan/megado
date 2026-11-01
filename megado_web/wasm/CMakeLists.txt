add_library(megado_api OBJECT api.c)

target_link_libraries(megado_api PUBLIC megado_core)

if(EMSCRIPTEN)
    set(MEGADO_WASM_OUTPUT "${PROJECT_SOURCE_DIR}/megado_web/app/src/wasm")

    set(MEGADO_WASM_EXPORTS
        _malloc
        _free
        _megado_create
        _megado_destroy
        _megado_load_rom
        _megado_reset
        _megado_rom_name
        _megado_status
        _megado_run_samples
        _megado_samples
        _megado_frame
        _megado_frame_width
        _megado_frame_height
        _megado_frame_pitch
        _megado_set_button
        _megado_snapshot_version
        _megado_snapshot_size
        _megado_snapshot_take
        _megado_snapshot_restore
    )
    list(JOIN MEGADO_WASM_EXPORTS "," MEGADO_WASM_EXPORTS)

    add_executable(megado_wasm $<TARGET_OBJECTS:megado_api>)

    set_target_properties(megado_wasm PROPERTIES
        OUTPUT_NAME megado
        SUFFIX ".js"
        RUNTIME_OUTPUT_DIRECTORY "${MEGADO_WASM_OUTPUT}"
    )

    target_link_libraries(megado_wasm PRIVATE megado_core)

    target_link_options(megado_wasm PRIVATE
        -sMODULARIZE=1
        -sEXPORT_ES6=1
        -sENVIRONMENT=web
        -sALLOW_MEMORY_GROWTH=1
        "-sEXPORTED_FUNCTIONS=[${MEGADO_WASM_EXPORTS}]"
        "-sEXPORTED_RUNTIME_METHODS=[HEAPU8,HEAP16,UTF8ToString]"
        "--emit-tsd=${MEGADO_WASM_OUTPUT}/megado.d.ts"
    )
endif()
