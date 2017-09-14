#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debugger.h"
#include "genesis.h"
#include "psg.h"
#include "ym2612.h"
#include "z80.h"
#include "z80_ops.h"

#ifdef DEBUG
#define LOG_Z80(...) printf(__VA_ARGS__)
#else
#define LOG_Z80(...)
#endif

static const uint32_t MASTER_CYCLES_PER_CLOCK = 15;

Z80* z80_make(struct Genesis* g) {
    Z80* z80 = calloc(1, sizeof(Z80));
    z80->genesis = g;
    return z80;
}

void z80_free(Z80* z) {
    free(z);
}

void z80_initialize(Z80* z) {
    z->remaining_master_cycles = 0;

    // Charles McDonald doc says the Z80 is running on reset, but Sonic expects
    // otherwise.
    z->pc = 0;
    z->running = 0;
}

uint8_t z80_step(Z80* z) {
    uint16_t instr_address = z->pc;
    FullyDecodedZ80Instruction* instr = z80_decode(z, instr_address);

    uint16_t opcode = z->ram[instr_address];
    z80_op op = z80_op_table[opcode];

    uint8_t cycles;
    if (op == NULL) {
        LOG_Z80("z80: Unknown opcode: %02x\n", opcode);
        z->pc = (instr_address + 1) % Z80_RAM_LENGTH;
        cycles = 4;
        debugger_post_z80(z->genesis->debugger, "NOP (Could not decode)", instr_address);
    } else {
        LOG_Z80("z80: %04x: %02x  %s\n", instr_address, opcode, z80_disasm_table[opcode]);
        cycles = (*op)(z);
        // Stub until instructions increase the PC themselves
        z->pc = (instr_address + instr->length) % Z80_RAM_LENGTH;
        if (cycles == 0) {
            LOG_Z80("z80: instruction took 0 cycles: %02x\n", opcode);
            cycles = 4;
        }
        debugger_post_z80(z->genesis->debugger, instr->mnemonics, instr_address);
    }

    if (instr != NULL) {
        fully_decoded_z80_instruction_free(instr);
    }
    return cycles;
}

FullyDecodedZ80Instruction* z80_decode(Z80* z, uint16_t address) {
    if (address < Z80_RAM_LENGTH) {
        uint16_t pc = address;
        uint16_t opcode = z->ram[pc++];
        // Adjust PC if opcode is 16bit
        if (opcode > 0xFF) { pc++; }
        DecodedZ80Instruction* i = &z80_disasm_table[opcode];
        // The disasm_table is not exhaustive
        if (i->length > 0) {
            // Prepare arguments
            uint16_t arg1 = 0;
            switch (i->arg1) {
            case Unsigned:
            case Signed:
            case Relative:
                arg1 = z->ram[pc++];
                break;

            case UnsignedWord:
                arg1 = z->ram[pc++] << 8;
                arg1 |= z->ram[pc++];
                break;
            default:
                break;
            }

            uint16_t arg2 = 0;
            switch (i->arg2) {
            case Unsigned:
            case Signed:
            case Relative:
                arg2 = z->ram[pc++];
                break;

            case UnsignedWord:
                arg2 = z->ram[pc++] << 8;
                arg2 |= z->ram[pc++];
                break;
            default:
                break;
            }

            FullyDecodedZ80Instruction* out = calloc(1, sizeof(FullyDecodedZ80Instruction));
            uint8_t bufsize = 100;
            out->length = i->length;
            out->mnemonics = calloc(bufsize + 1, sizeof(char));

            // XXX: A bit dumb, but I don't have sprintf.apply here maybe use
            // var_list?
            if (i->arg1 == Relative) {
                snprintf(out->mnemonics, bufsize, i->mnemonics_fmt, (int8_t)arg1, address + (int8_t)arg1);
            } else if (i->arg2 == Relative) {
                snprintf(out->mnemonics, bufsize, i->mnemonics_fmt, (int8_t)arg2, address + (int8_t)arg2);
            } else if (i->arg1 > 0 && i->arg2 > 0) {
                snprintf(out->mnemonics, bufsize, i->mnemonics_fmt, arg1, arg2);
            } else if (i->arg1 > 0) {
                snprintf(out->mnemonics, bufsize, i->mnemonics_fmt, arg1);
            } else if (i->arg2 > 0) {
                snprintf(out->mnemonics, bufsize, i->mnemonics_fmt, arg2);
            } else {
                strcpy(out->mnemonics, i->mnemonics_fmt);
            }

            return out;
        }
    }
    return NULL;
}

void fully_decoded_z80_instruction_free(FullyDecodedZ80Instruction* i) {
    free(i->mnemonics);
    free(i);
}

void z80_run_cycles(Z80* z, uint32_t cycles) {
    if (z->resetting || !z->running) return;

    z->remaining_master_cycles += cycles;

    while (z->remaining_master_cycles > 0) {
        z->remaining_master_cycles -= z80_step(z) * MASTER_CYCLES_PER_CLOCK;
    }
}

void z80_reset(Z80* z, uint8_t rst) {
    if (rst == 0) {
        z->pc = 0;
        z->remaining_master_cycles = 0;
        z->resetting = true;
        LOG_Z80("z80: RESET ON\n");
    } else {
        z->resetting = false;
        LOG_Z80("z80: RESET OFF\n");
    }
}

void z80_bus_req(Z80 *z, uint8_t req) {
    if (req) {
        z->running = 0;
        LOG_Z80("z80: BUSREQ ON\n");
    } else {
        z->running = 1;
        LOG_Z80("z80: BUSREQ OFF\n");
    }
}

uint8_t z80_bus_ack(Z80 *z) {
    // Can't write to the Z80 memory until it has stopped running
    if (z->running) {
        LOG_Z80("z80: BUSACK BUSY\n");
        return 1;
    } else {
        LOG_Z80("z80: BUSACK FREE\n");
        return 0;
    }
}

uint8_t z80_read(Z80 *z, uint16_t address) {
    LOG_Z80("z80: READ @ %04x\n", address);

    // Z80 RAM
    if (address < 0x2000) {
        return z->ram[address];
    }

    // RAM mirror
    else if (address < 0x4000) {
        return z->ram[address - 0x2000];

    }

    // YM2612
    else if (address <= 0x4003) {
        return ym2612_read(z->genesis->ym2612, address);
    }

    else {
        return 0xFF;
    }
}

void z80_write(Z80 *z, uint16_t address, uint8_t value) {
    LOG_Z80("z80: WRITE @ %04x : %02x\n", address, value);

    // Z80 RAM
    if (address < 0x2000) {
        z->ram[address] = value;
    }

    // RAM mirror
    else if (address < 0x4000) {
        z->ram[address - 0x2000] = value;
    }

    // YM2612
    else if (address <= 0x4003) {
        ym2612_write(z->genesis->ym2612, address, value);
    }

    // PSG access from Z80
    else if (address == 0x7f11) {
        printf("Z80 writes to PSG\n");
        psg_write(z->genesis->psg, value);
    }
}

uint8_t z80_op_nop(Z80 *z) {
    return 4;
}
