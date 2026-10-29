#include <stdio.h>
#include <stdlib.h>

#include "megado_core/genesis.h"
#include "megado_core/snapshot.h"
#include "snapshot.h"

#define WRITE_SNAPSHOT_NAME(BUFFER, TITLE, SLOT) sprintf(BUFFER, "%s.snapshot%d", TITLE, SLOT)

SnapshotMetadata* snapshot_save(struct Genesis* g, uint8_t slot)
{
    SnapshotMetadata* metadata = calloc(1, sizeof(SnapshotMetadata));
    genesis_get_rom_name(g, metadata->game);
    metadata->date = time(NULL);
    metadata->version = SNAPSHOT_VERSION;

    Snapshot* snapshot = snapshot_take(g);

    char file_name[70];
    WRITE_SNAPSHOT_NAME(file_name, metadata->game, slot);

    printf("Saving snapshot %s...\n", file_name);
    FILE* file = fopen(file_name, "wb");
    if (!file)
    {
        printf("Cannot open file \"%s\"", file_name);
        return NULL;
    }

    fwrite(metadata, sizeof(SnapshotMetadata), 1, file);
    fwrite(snapshot, sizeof(Snapshot), 1, file);
    fclose(file);

    return metadata;
}

void snapshot_load(struct Genesis* g, uint8_t slot)
{
    char rom_name[49];
    genesis_get_rom_name(g, rom_name);

    char file_name[70];
    WRITE_SNAPSHOT_NAME(file_name, rom_name, slot);

    printf("Loading snapshot %s...\n", file_name);

    FILE* file = fopen(file_name, "rb");
    if (!file)
    {
        printf("Cannot open file \"%s\"", file_name);
        return;
    }

    Snapshot snapshot;
    fseek(file, sizeof(SnapshotMetadata), SEEK_SET); // Skip the header
    fread(&snapshot, sizeof(Snapshot), 1, file);
    fclose(file);

    snapshot_restore(g, &snapshot);
}

void snapshots_preload(struct Genesis* g, SnapshotMetadata* snapshots[])
{
    char rom_name[49];
    genesis_get_rom_name(g, rom_name);

    for (uint8_t slot = 0; slot < SNAPSHOT_SLOTS; ++slot)
    {
        char file_name[70];
        WRITE_SNAPSHOT_NAME(file_name, rom_name, slot);

        FILE* file = fopen(file_name, "rb");

        if (!file)
        {
            snapshots[slot] = NULL;
            continue;
        }

        SnapshotMetadata* metadata = calloc(1, sizeof(SnapshotMetadata));
        fread(metadata, sizeof(SnapshotMetadata), 1, file);
        fclose(file);

        printf("Snapshot found: %s\n", file_name);

        if (metadata->version != SNAPSHOT_VERSION)
        {
            printf("Incompatible snapshot version (loaded version is %d, current version is %d)", metadata->version, SNAPSHOT_VERSION);

            snapshots[slot] = NULL;
            snapshot_metadata_free(metadata);

            continue;
        }

        snapshots[slot] = metadata;
    }
}

void snapshot_metadata_free(SnapshotMetadata* s)
{
    free(s);
}
