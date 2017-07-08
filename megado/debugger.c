#include <GLFW/glfw3.h>
#include <stdlib.h>

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

void debugger_post_m68k(Debugger* d)
{
    // Append the last instruction's address to the program log
    if (d->genesis->settings->show_cpu_log)
    {
        d->m68k_log_cursor = (d->m68k_log_cursor + 1) % M68K_LOG_LENGTH;
        d->m68k_log_addresses[d->m68k_log_cursor] = d->genesis->m68k->instruction_address;
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
