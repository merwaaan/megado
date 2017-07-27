#pragma once

#include <stdint.h>

struct Genesis;

typedef struct PSG {

} PSG;

PSG* psg_make(struct Genesis*);
void psg_free(PSG*);
void psg_write(PSG*, uint8_t);
