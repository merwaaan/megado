#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "bit_utils.h"

struct Condition;
struct Instruction;
struct M68k;
struct Operand;

typedef int (InstructionFunc)(struct Instruction*);

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

    // Instruction length in bytes
    uint8_t base_length;

    // Total instruction length (take into account the operands' length)
    uint8_t total_length;

    // Base execution time (fixed part of the instruction's timing)
    //
    // The implementation will return an additional context-dependent
    // execution time that must be added to obtain the total execution
    // time.
    //
    // e.g. for ROL.b, base time is 6
    //                 total time is 6+2n where n is the number of bits to rotate
    //      -> the implementation will return 2n
    uint8_t base_cycles;

    // Some instructions require extra data
    union
    {
        struct Condition* condition; // Branching condition index (see conditions.h)
    };
} Instruction;

// TODO just make it a char*?
typedef struct DecodedInstruction {
    char mnemonics[50];
} DecodedInstruction;

Instruction* instruction_make(struct M68k* context, char* name, InstructionFunc func);
void instruction_free(Instruction*);

// Generate the appropriate instruction from an opcode
Instruction* instruction_generate(struct M68k* context, uint16_t opcode);

// Check if an instruction is fully formed, ie. a function have been
// assigned and all operands have been setup
bool instruction_valid(Instruction*);

// Execute the given instruction.
// Returns the elapsed cycles.
int instruction_execute(Instruction*);

/*
 * Instruction generators
 */

#define DEFINE_INSTR(name) Instruction* gen_ ## name (uint16_t opcode, struct M68k* context)

// Bit-wise operations
DEFINE_INSTR(bchg);
DEFINE_INSTR(bchg_imm);
DEFINE_INSTR(bclr);
DEFINE_INSTR(bclr_imm);
DEFINE_INSTR(bset);
DEFINE_INSTR(bset_imm);
DEFINE_INSTR(btst);
DEFINE_INSTR(btst_imm);

// Logic operations
DEFINE_INSTR(and);
DEFINE_INSTR(andi);
DEFINE_INSTR(eor);
DEFINE_INSTR(eori);
DEFINE_INSTR(or);
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
DEFINE_INSTR(cmp);
DEFINE_INSTR(ext);
DEFINE_INSTR(mulu);
DEFINE_INSTR(muls);
DEFINE_INSTR(subq);

// Shit & rotate
DEFINE_INSTR(asd);
DEFINE_INSTR(lsd);
DEFINE_INSTR(rod);
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
DEFINE_INSTR(move_from_ccr);
DEFINE_INSTR(move_to_ccr);
DEFINE_INSTR(move_from_sr);
DEFINE_INSTR(move_to_sr);
DEFINE_INSTR(move_usp);
DEFINE_INSTR(pea);

// Program control
DEFINE_INSTR(bcc);
DEFINE_INSTR(bra);
DEFINE_INSTR(bsr);
DEFINE_INSTR(dbcc);
DEFINE_INSTR(jmp);
DEFINE_INSTR(jsr);
DEFINE_INSTR(nop);
//DEFINE_INSTR(rtd);
//DEFINE_INSTR(rtr);
DEFINE_INSTR(rts);