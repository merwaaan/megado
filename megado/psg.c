#include <stdio.h>
#include <stdlib.h>

#include "psg.h"

PSG* psg_make(struct Genesis* g) {
    return calloc(1, sizeof(PSG));
}

void psg_free(PSG* p) {
    free(p);
}

void psg_write(PSG* p, uint8_t value) {
    printf("Unhandled PSG write: %x\n", value);
}
