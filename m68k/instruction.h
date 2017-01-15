#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "bit_utils.h"

struct Condition;
struct Instruction;
struct M68k;
struct Operand;

typedef void (InstructionFunc)(struct Instruction*);

typedef struct Instruction {
    char* name;

    // The M68000 instance that the instruction is bound to
    struct M68k* context;

    // Implementation
    InstructionFunc* func;

    // Operands
    struct Operand* src;
    struct Operand* dst;

    // Size of the operation (byte, word, long)
    Size size;

    // Instruction length in bytes (depends on the operands' length)
    uint8_t length;

    // Some instructions require extra data
    union
    {
        struct Condition* condition; // Branching condition index (see conditions.h)
    };
} Instruction;

// TODO just make it a char*?
typedef struct DecodedInstruction {
    char* mnemonics;
} DecodedInstruction;

Instruction* instruction_make(struct M68k* context, char* name, InstructionFunc func);
void instruction_free(Instruction* instr);

// Generate the appropriate instruction from an opcode
Instruction* instruction_generate(struct M68k* context, uint16_t opcode);

// Check if an instruction is fully formed, ie. a function have been
// assigned and all operands have been setup
bool instruction_valid(Instruction* instr);

/*
 * Instruction implementations.
 */

#define DEFINE_INSTR(name) Instruction* gen_ ## name (uint16_t opcode, struct M68k* context)

// Bit-wise operations
DEFINE_INSTR(bchg);
DEFINE_INSTR(bclr);
DEFINE_INSTR(bset);
DEFINE_INSTR(btst);

// Logic operations
DEFINE_INSTR(and);
DEFINE_INSTR(andi);
DEFINE_INSTR(eor);
DEFINE_INSTR(eori);
DEFINE_INSTR(or );
DEFINE_INSTR(ori);
DEFINE_INSTR(not);
DEFINE_INSTR(scc);
DEFINE_INSTR(tst);

// Arithmetic operations
DEFINE_INSTR(add);
DEFINE_INSTR(adda);
DEFINE_INSTR(addi);
DEFINE_INSTR(addq);
DEFINE_INSTR(addx);
DEFINE_INSTR(clr);
DEFINE_INSTR(ext);
DEFINE_INSTR(mulu);
DEFINE_INSTR(muls);

// Shit & rotate
DEFINE_INSTR(lsX);
DEFINE_INSTR(swap);

// Data transfer
DEFINE_INSTR(exg);
DEFINE_INSTR(lea);
DEFINE_INSTR(move);
DEFINE_INSTR(move);
DEFINE_INSTR(movea);
DEFINE_INSTR(movem);
DEFINE_INSTR(moveq);
DEFINE_INSTR(movep);
DEFINE_INSTR(move_usp);
DEFINE_INSTR(pea);

// Program control
DEFINE_INSTR(bcc);
// TODO DEFINE_INSTR(dbcc);
DEFINE_INSTR(bra);
DEFINE_INSTR(bsr);
DEFINE_INSTR(jmp);
DEFINE_INSTR(jsr);
//DEFINE_INSTR(rtd);
//DEFINE_INSTR(rtr);
DEFINE_INSTR(rts);