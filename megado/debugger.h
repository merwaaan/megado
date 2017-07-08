#pragma once

#include <stdbool.h>
#include <stdint.h>

#define M68K_LOG_LENGTH 10

#define REWIND_BUFFER_LENGTH 100
#define REWIND_SAVE_INTERVAL 0.1
#define REWIND_PLAY_INTERVAL 0.05

struct DecodedInstruction;
struct Genesis;
struct Snapshot;

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
    // TODO

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

void debugger_post_m68k(Debugger*);
void debugger_post_frame(Debugger*);

// Restores the most recent rewinding snapshot.
// Returns false if there is no more snapshot no restore.
bool debugger_rewind(Debugger* d);
