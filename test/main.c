#include <GLFW/glfw3.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <megado/genesis.h>
#include <megado/psg.h>
#include <megado/settings.h>
#include <megado/ym2612.h>
#include <megado/m68k/instruction.h>
#include <megado/m68k/m68k.h>

void run(Genesis* g, char* path)
{
    genesis_load_rom_file(g, path);

// Enable debug windows on CI to catch more bugs
#ifdef TRAVIS
    g->settings->vsync = true;
    g->settings->video_scale = 1f;
    g->settings->rewinding_enabled = true;
    g->settings->emulation_speed = 1f; // FIXME: we should as fast as we can on
                                       // CI, as a fixed speed rate may already
                                       // be too much for the VM.

    g->settings->show_m68k_registers = true;
    g->settings->show_m68k_disassembly = true;
    g->settings->show_m68k_log = true;
    g->settings->show_z80_registers = true;
    g->settings->show_z80_disassembly = true;
    g->settings->show_z80_log = true;
    g->settings->show_vdp_registers = true;
    g->settings->show_vdp_palettes = true;
    g->settings->show_vdp_patterns = true;
    g->settings->show_vdp_planes = true;
    g->settings->show_vdp_sprites = true;
    g->settings->show_psg_registers = true;
    g->settings->show_ym2612_registers = true;
    g->settings->show_rom = true;
    g->settings->show_ram = true;
    g->settings->show_vram = true;
    g->settings->show_vsram = true;
    g->settings->show_cram = true;
    g->settings->show_metrics = true;
#endif

    double start = glfwGetTime();

    while (g->status != Status_Quitting) {
        genesis_update(g);

// Automatically exit after 5 minutes on continuous integration
#ifdef TRAVIS
        if (glfwGetTime() - start > 300) break;
#endif
    }
}

int main(int argc, char **argv)
{
    // Generate the instruction table
    opcode_table = calloc(0x10000, sizeof(Instruction*));
    for (int opcode = 0; opcode < 0x10000; ++opcode)
        opcode_table[opcode] = instruction_generate(opcode);

    Genesis* g = genesis_make();

    if (argc < 2) {
        printf("No ROM specified, using default\n");
        run(g, "../browser/test.bin");
    }
    else {
        run(g, argv[1]);
    }

    genesis_free(g);

    for (int opcode = 0; opcode < 0x10000; ++opcode)
        instruction_free(opcode_table[opcode]);
    free(opcode_table);

    return 0;
}
