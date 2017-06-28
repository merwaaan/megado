#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "snapshot.h"

void snapshot_save(Genesis* g, uint8_t slot)
{
    SnapshotMetadata metadata;
    strcpy(metadata.game, "test");
    metadata.date = time(NULL);
    metadata.version = SNAPSHOT_VERSION;

    Snapshot snapshot;
    //snapshot.memory = NULL;
    snapshot.m68k = *g->m68k;
    snapshot.z80 = *g->z80;
    snapshot.vdp = *g->vdp;

    char* name = SNAPSHOT_NAME(metadata.game, slot);

    printf("Saving snapshot %s...\n", name);
    FILE* file = fopen(name, "w");
    if (!file)
    {
        printf("Cannot open file \"%s\"", name);
        return;
    }

    fwrite(&metadata, sizeof(SnapshotMetadata), 1, file);
    fwrite(&snapshot, sizeof(Snapshot), 1, file);
    fclose(file);
}

void snapshot_load(Genesis* g, uint8_t slot)
{
    char* name = SNAPSHOT_NAME("test", slot);
    printf("Loading snapshot %s...\n", name);

    FILE* file = fopen(name, "r");
    if (!file)
    {
        printf("Cannot open file \"%s\"", name);
        return;
    }

    Snapshot s;
    fseek(file, sizeof(SnapshotMetadata), SEEK_SET); // Skip the header
    fread(&s, sizeof(Snapshot), 1, file);
    fclose(file);

    // TODO mem
    *g->m68k = s.m68k;
    *g->z80 = s.z80;
    *g->vdp = s.vdp;
}

SnapshotMetadata* snapshots_preload(Genesis* g)
{
    SnapshotMetadata* snapshots = calloc(SNAPSHOT_SLOTS, sizeof(SnapshotMetadata));

    for (uint8_t slot = 0; slot < SNAPSHOT_SLOTS; ++slot)
    {
        char* name = SNAPSHOT_NAME("test" , slot);

        FILE* file = fopen(name, "r");
        if (!file)
            continue;

        SnapshotMetadata metadata;
        fread(&metadata, sizeof(SnapshotMetadata), 1, file);
        fclose(file);

        printf("Snapshot found: %s\n", name);

        if (metadata.version < SNAPSHOT_VERSION)
        {
            printf("Snapshot is out of date (loaded version is %d, current version is %d)", metadata.version, SNAPSHOT_VERSION);
            continue;
        }

        snapshots[slot] = metadata;
    }

    return snapshots;
}
