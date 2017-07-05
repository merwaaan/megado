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
    memcpy(snapshot.ram, g->memory + 0xFF0000, 0x10000 * sizeof(uint8_t));
    snapshot.m68k = *g->m68k;
    snapshot.z80 = *g->z80;
    snapshot.vdp = *g->vdp;

    char file_name[60];
    sprintf(file_name, "%s %d.snapshot", metadata.game, slot);

    printf("Saving snapshot %s...\n", file_name);
    FILE* file = fopen(file_name, "wb");
    if (!file)
    {
        printf("Cannot open file \"%s\"", file_name);
        return;
    }

    fwrite(&metadata, sizeof(SnapshotMetadata), 1, file);
    fwrite(&snapshot, sizeof(Snapshot), 1, file);
    fclose(file);
}

void snapshot_load(Genesis* g, uint8_t slot)
{
    char file_name[60];
    sprintf(file_name, "%s %d.snapshot", "test", slot);

    printf("Loading snapshot %s...\n", file_name);

    FILE* file = fopen(file_name, "rb");
    if (!file)
    {
        printf("Cannot open file \"%s\"", file_name);
        return;
    }

    Snapshot s;
    fseek(file, sizeof(SnapshotMetadata), SEEK_SET); // Skip the header
    fread(&s, sizeof(Snapshot), 1, file);
    fclose(file);

    memcpy(g->memory + 0xFF0000, &s.ram, 0x10000 * sizeof(uint8_t));
    *g->m68k = s.m68k;
    *g->z80 = s.z80;
    *g->vdp = s.vdp;

    // Rebind internal pointers
    g->m68k->genesis = g;
    g->vdp->genesis = g;
}

SnapshotMetadata* snapshots_preload(Genesis* g)
{
    SnapshotMetadata* snapshots = calloc(SNAPSHOT_SLOTS, sizeof(SnapshotMetadata));

    for (uint8_t slot = 0; slot < SNAPSHOT_SLOTS; ++slot)
    {
        char* name = SNAPSHOT_NAME("test", slot);

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
