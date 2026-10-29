#include <stdlib.h>
#include <string.h>

#include "genesis.h"
#include "snapshot.h"

Snapshot* snapshot_take(struct Genesis* g)
{
    Snapshot* snapshot = calloc(1, sizeof(Snapshot));
    memcpy(snapshot->ram, g->ram, 0x10000 * sizeof(uint8_t));
    snapshot->m68k = *g->m68k;
    snapshot->z80 = *g->z80;
    snapshot->vdp = *g->vdp;
    snapshot->psg = *g->psg;
    snapshot->ym2612 = *g->ym2612;
    return snapshot;
}

void snapshot_restore(struct Genesis* g, Snapshot* s)
{
    uint8_t* vdp_buffer = g->vdp->output_buffer;

    // Copy the snapshot data

    memcpy(g->ram, &s->ram, 0x10000 * sizeof(uint8_t));
    memcpy(g->m68k, &s->m68k, sizeof(M68k));
    memcpy(g->z80, &s->z80, sizeof(Z80));
    memcpy(g->vdp, &s->vdp, sizeof(Vdp));
    memcpy(g->psg, &s->psg, sizeof(PSG));
    memcpy(g->ym2612, &s->ym2612, sizeof(YM2612));

    // Rebind internal pointers

    g->m68k->genesis = g;
    g->z80->genesis = g;
    g->vdp->genesis = g;
    g->vdp->output_buffer = vdp_buffer;
    g->psg->genesis = g;
    g->ym2612->genesis = g;
}
