#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "genesis.h"
#include "debugger.h"
#include "m68k/bit_utils.h"
#include "m68k/m68k.h"
#include "settings.h"
#include "snapshot.h"

Debugger* debugger_make(Genesis* g)
{
    Debugger* d = calloc(1, sizeof(Debugger));
    d->genesis = g;
    return d;
}

void debugger_free(Debugger* d)
{
    free(d);
}

void debugger_initialize(Debugger *d) {
    // Reset log entries
    memset(d->m68k_log_addresses, 0, M68K_LOG_LENGTH * sizeof(d->m68k_log_addresses[0]));
    d->m68k_log_cursor = 0;

    memset(d->z80_log_instrs, 0, Z80_LOG_LENGTH * sizeof(d->z80_log_instrs[0]));
    d->z80_log_cursor = 0;
}

void debugger_preload(Debugger* d)
{
    char name[49];
    genesis_get_rom_name(d->genesis, name);
    d->breakpoints = settings_get_or_create_breakpoints(d->genesis->settings, name);
}
void debugger_post_m68k(Debugger* d)
{
    // Append the last instruction's address to the program log
    if (d->genesis->settings->show_m68k_log)
    {
        d->m68k_log_cursor = (d->m68k_log_cursor + 1) % M68K_LOG_LENGTH;
        d->m68k_log_addresses[d->m68k_log_cursor] = d->genesis->m68k->instruction_address;
    }
}

void debugger_post_z80(Debugger *d, DecodedZ80Instruction* instr, uint16_t address) {
    if (d->genesis->settings->show_z80_log) {
        d->z80_log_cursor = (d->z80_log_cursor + 1) % Z80_LOG_LENGTH;
        d->z80_log_instrs[d->z80_log_cursor].address = address;
        if (instr == NULL) {
            d->z80_log_instrs[d->z80_log_cursor].mnemonics = NULL;
        } else {
            d->z80_log_instrs[d->z80_log_cursor].mnemonics = instr->mnemonics;
        }
    }
}

void debugger_post_frame(Debugger* d)
{
    // Regularly take snapshots
    if (d->genesis->settings->rewinding_enabled)
    {
        double now = glfwGetTime();
        if (now - d->rewinding_last_save >= REWIND_SAVE_INTERVAL)
        {
            d->rewinding_cursor = (d->rewinding_cursor + 1) % REWIND_BUFFER_LENGTH;

            // Deallocate overwritten snapshots
            if (d->rewinding_snapshots[d->rewinding_cursor] != NULL)
                free(d->rewinding_snapshots[d->rewinding_cursor]);

            d->rewinding_snapshots[d->rewinding_cursor] = snapshot_take(d->genesis);

            d->rewinding_last_save = now;
        }
    }
}

void debugger_toggle_breakpoint(Debugger* d, uint32_t address)
{
    // Toggle existing breakpoints
    for (int i = 0; i < BREAKPOINTS_COUNT; ++i)
        if (d->breakpoints[i].address == address)
        {
            d->breakpoints[i].enabled = !d->breakpoints[i].enabled;
            return;
        }

    // Otherwise, use the first free slot
    for (int i = 0; i < BREAKPOINTS_COUNT; ++i)
        if (!d->breakpoints[i].enabled)
        {
            d->breakpoints[i].enabled = true;
            d->breakpoints[i].address = address;
            return;
        }

    printf("Warning, no free slots for a new breakpoint at %0X", address); // TODO would be nice to output warnings/errors via the UI too
}

Breakpoint* debugger_get_breakpoint(Debugger* d, uint32_t address)
{
    for (int i = 0; i < BREAKPOINTS_COUNT; ++i)
        if (d->breakpoints[i].enabled && d->breakpoints[i].address == address)
            return &d->breakpoints[i];

    return NULL;
}

bool debugger_rewind(Debugger* d)
{
    double now = glfwGetTime();
    if (now - d->rewinding_last_restore >= REWIND_PLAY_INTERVAL)
    {
        d->rewinding_cursor = UMOD(d->rewinding_cursor, REWIND_BUFFER_LENGTH);

        Snapshot* snapshot = d->rewinding_snapshots[d->rewinding_cursor];
        if (snapshot == NULL)
            return false;

        snapshot_restore(d->genesis, snapshot);

        // Deallocate restored snapshots
        free(snapshot);
        d->rewinding_snapshots[d->rewinding_cursor] = NULL;

        d->rewinding_last_restore = now;
        d->rewinding_cursor--;
    }

    return true;
}
