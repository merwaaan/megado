#include <stdio.h>
#include <stdlib.h>

#include "psg.h"

void psg_write_latch(PSG*, uint8_t);
void psg_write_data(PSG*, uint8_t);

PSG* psg_make(struct Genesis* g) {
    return calloc(1, sizeof(PSG));
}

void psg_free(PSG* p) {
    free(p);
}

// Entry point for other chip to communicate with the PSG
void psg_write(PSG* p, uint8_t value) {
    if (value & 0x80) {
        p->latched_channel = value >> 5;
        p->latched_register = value >> 4;
        psg_write_latch(p, value);
    } else {
        psg_write_data(p, value);
    }

    for (int i=0; i < 3; ++i) {
        printf("chan%d: %x (vol) %x (tone)\n", i, p->square[i].volume, p->square[i].tone);
    }
    printf("noise: %x (vol) %x (noise)\n", p->noise.volume, p->noise.noise);
}

// Write low nibble to the currently latched channel & register
void psg_write_latch(PSG* p, uint8_t value) {
    value = value & 0xf;
    switch (p->latched_channel) {
    case 0:
    case 1:
    case 2:
        if (p->latched_register) {
            p->square[p->latched_channel].volume = value;
        } else {
            p->square[p->latched_channel].tone = value;
        }
        break;

    case 3:
        if (p->latched_register) {
            p->noise.volume = value;
        } else {
            p->noise.noise = value;
        }
        break;
    }
}

// Write 6bit value to the currently latched channel & register
void psg_write_data(PSG* p, uint8_t value) {
    value = value & 0x3f;
    switch (p->latched_channel) {
    case 0:
    case 1:
    case 2:
        if (p->latched_register) {
            p->square[p->latched_channel].volume = value;
        } else {
            p->square[p->latched_channel].tone = value << 4;
        }
        break;

    case 3:
        if (p->latched_register) {
            p->noise.volume = value;
        } else {
            p->noise.noise = value;
        }
        break;
    }
}
