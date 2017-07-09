#pragma once

#include <stdbool.h>
#include <stdint.h>

#define M68K_LOG_LENGTH 25

#define BREAKPOINTS_COUNT 3

#define REWIND_BUFFER_LENGTH 100
#define REWIND_SAVE_INTERVAL 0.1
#define REWIND_PLAY_INTERVAL 0.05

struct DecodedInstruction;
struct Genesis;
struct Snapshot;

typedef struct Breakpoint
{
    bool enabled;
    uint32_t address;
    // TODO hit counter could be useful
} Breakpoint;

typedef struct Debugger
{
    struct Genesis* genesis;

    // Move metrics here?
    // ...

    // Program log
    //
    // Note: We store the instructions' addresses because decoding
    // the last instruction at each step is prohibitively costly.
    // The downside of this approach is that the memory at an
    // instruction's address might have changed when it is decoded
    // later.
    uint32_t m68k_log_addresses[M68K_LOG_LENGTH];
    uint16_t m68k_log_cursor;

    // Breakpoints
    Breakpoint* breakpoints; // Points to the data stored in settings
    Breakpoint* active_breakpoint; // The breakpoint currently blocking the emulation

    // Watchpoints
    // TODO

    // Rewinding
    struct Snapshot* rewinding_snapshots[REWIND_BUFFER_LENGTH];
    int rewinding_cursor;
    double rewinding_last_save;
    double rewinding_last_restore;

} Debugger;

Debugger* debugger_make(struct Genesis*);
void debugger_free(Debugger*);

void debugger_preload(Debugger*);

void debugger_post_m68k(Debugger*);
void debugger_post_frame(Debugger*);

void debugger_toggle_breakpoint(Debugger*, uint32_t address);
Breakpoint* debugger_get_breakpoint(Debugger*, uint32_t address);

// Restores the most recent rewinding snapshot.
// Returns false if there is no more snapshot no restore.
bool debugger_rewind(Debugger* d);
