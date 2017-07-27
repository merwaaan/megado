#pragma once

#include <stdbool.h>
#include <stdint.h>

#define Z80_RAM_LENGTH 0x2000

typedef struct Z80 {
    struct Genesis* genesis;

    int32_t left_cycles;

    // Registers
    struct {
        union {
            struct {
#ifdef BIG_ENDIAN
                uint8_t a;
                union {
                    uint8_t f;
                    struct {
                        uint8_t cf : 1;
                        uint8_t nf : 1;
                        uint8_t pf : 1;
                        uint8_t xf : 1;
                        uint8_t hf : 1;
                        uint8_t yf : 1;
                        uint8_t zf : 1;
                        uint8_t sf : 1;
                    };
                };
                uint8_t b, c, d, e, h, l;
#else
                union {
                    uint8_t f;
                    struct {
                        uint8_t cf : 1;
                        uint8_t nf : 1;
                        uint8_t pf : 1;
                        uint8_t xf : 1;
                        uint8_t hf : 1;
                        uint8_t yf : 1;
                        uint8_t zf : 1;
                        uint8_t sf : 1;
                    };
                };
                uint8_t a, c, b, e, d, l, h;
#endif
            };
            struct {
                uint16_t af, bc, de, hl;
            };
        };

        uint8_t i, r;
        uint16_t pc, sp, ix, iy;
        uint16_t af_, bc_, de_, hl_; // shadow registers
    };

    bool running;                 // Whether the Z80 is executing instructions
    bool resetting;               // Whether the Z80 is being reset

    uint8_t ram[Z80_RAM_LENGTH];
} Z80;

typedef enum {
    None,
    Unsigned,
    UnsignedWord,
    Signed,
    Relative,
} Z80MnemonicArgType;

typedef struct DecodedZ80Instruction {
    uint8_t length;
    char* mnemonics_fmt;
    Z80MnemonicArgType arg1;
    Z80MnemonicArgType arg2;
} DecodedZ80Instruction;

typedef struct FullyDecodedZ80Instruction {
    uint8_t length;
    char* mnemonics;
} FullyDecodedZ80Instruction;

Z80* z80_make(struct Genesis*);
void z80_free(Z80*);

void z80_initialize(Z80*);

uint8_t z80_step(Z80*);
void z80_run_cycles(Z80*, int);

void z80_bus_req(Z80*, uint8_t);
uint8_t z80_bus_ack(Z80*);
void z80_reset(Z80*, uint8_t);

uint8_t z80_read(Z80*, uint16_t);
void z80_write(Z80 *, uint16_t, uint8_t);

FullyDecodedZ80Instruction* z80_decode(Z80*, uint16_t);
void fully_decoded_z80_instruction_free(FullyDecodedZ80Instruction*);
