#pragma once

#include <stdint.h>
#include <time.h>

struct Genesis;

#define SNAPSHOT_SLOTS 3

typedef struct SnapshotMetadata
{
    char game[48]; // Game name, extracted from the header
    time_t date;
    uint8_t version;
} SnapshotMetadata;

// Saves/Loads a snapshot in the given slot for the game currently being executed
SnapshotMetadata* snapshot_save(struct Genesis*, uint8_t slot);
void snapshot_load(struct Genesis*, uint8_t slot);

// Gets the metadata from saved snapshots of the game currently being executed
void snapshots_preload(struct Genesis*, SnapshotMetadata*[]);

void snapshot_metadata_free(SnapshotMetadata*);
