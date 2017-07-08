#pragma once

#include <stdint.h>
#include <time.h>

#include "genesis.h"
#include "m68k/m68k.h"
#include "vdp.h"
#include "z80.h"

#define SNAPSHOT_SLOTS 3
#define SNAPSHOT_NAME(NAME, SLOT) NAME #SLOT ".snapshot"

// Bump that version number when changing the format or the structure of the
// underlying data in order to discard out of date snapshots.
#define SNAPSHOT_VERSION 0

// State of the emulator at a given time.
// Can be saved to/loaded from file.
//
// Note: This is most likely not portable!
//       Snapshots saved on a machine might not load on another architecture due to endianness or padding.
//
// TODO use a portable serialization lib
// TODO use a really unique identifier for the snapshot's name (right now, could clash between rom versions)

typedef struct Snapshot
{
    uint8_t ram[0x10000];
    M68k m68k; // TODO remove breakpoints
    Z80 z80;
    Vdp vdp;

} Snapshot;

typedef struct SnapshotMetadata
{
    char game[48]; // Game name, extracted from the header
    time_t date;
    uint8_t version;
} SnapshotMetadata;

// Takes/Restores a snapshot for the game currently being executed
Snapshot* snapshot_take(Genesis*);
void snapshot_restore(Genesis*, Snapshot*);

// Saves/Loads a snapshot in the given slot for the game currently being executed
SnapshotMetadata* snapshot_save(Genesis*, uint8_t slot);
void snapshot_load(Genesis*, uint8_t slot);

// Gets the metadata from saved snapshots of the game currently being executed
void snapshots_preload(Genesis*, SnapshotMetadata*[]);
