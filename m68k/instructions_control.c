#include <stdbool.h>
#include <stdlib.h>

#include "bit_utils.h"
#include "instruction.h"
#include "instructions_arithmetic.h"
#include "m68k.h"
#include "operands.h"

void bcc(Instruction* i)
{
    if (GET(i->operands[1]))
        m68k_jump(i->context->pc + GET(i->operands[0]));
}

Instruction* gen_bcc(uint16_t opcode, M68k* m) // TODO factor with bra?
{
    Instruction* i = calloc(1, sizeof(Instruction));
    i->context = m;
    i->name = "BCC";
    i->func = bcc;

    i->operands = calloc(2, sizeof(Operand));
    i->operand_count = 2;
    i->operands[1] = operand_make_condition(FRAGMENT(opcode, 11, 8), i);

    int displacement = FRAGMENT(opcode, 7, 0);
    if (displacement == 0)
        i->operands[0] = operand_make_extension(16, i);
    else if (displacement == 0xFF)
        i->operands[0] = operand_make_extension(32, i);
    else
        i->operands[0] = operand_make_immediate(FRAGMENT(opcode, 7, 0), i);

    return i;
}

void bra(Instruction* i)
{
    m68k_jump(i->context->pc + GET(i->operands[0]));
}

Instruction* gen_bra(uint16_t opcode, M68k* m)
{
    Instruction* i = calloc(1, sizeof(Instruction));
    i->context = m;
    i->name = "BRA";
    i->func = bra;

    i->operands = calloc(1, sizeof(Operand));
    i->operand_count = 1;

    int displacement = FRAGMENT(opcode, 7, 0);
    if (displacement == 0)
        i->operands[0] = operand_make_extension(16, i);
    else if (displacement == 0xFF)
        i->operands[0] = operand_make_extension(32, i);
    else
        i->operands[0] = operand_make_immediate(FRAGMENT(opcode, 7, 0), i);

    return i;
}

void bsr(Instruction* i)
{
    m68k_push(i->context->pc);
    m68k_jump(i->context->pc + GET(i->operands[0]));
}

Instruction* gen_bsr(uint16_t opcode, M68k* m)
{
    Instruction* i = calloc(1, sizeof(Instruction));
    i->context = m;
    i->name = "BSR";
    i->func = bsr;

    i->operands = calloc(1, sizeof(Operand));
    i->operand_count = 1;

    int displacement = FRAGMENT(opcode, 7, 0);
    if (displacement == 0)
        i->operands[0] = operand_make_extension(16, i);
    else if (displacement == 0xFF)
        i->operands[0] = operand_make_extension(32, i);
    else
        i->operands[0] = operand_make_immediate(FRAGMENT(opcode, 7, 0), i);

    return i;
}

void jmp(Instruction* i)
{
    i->context->pc = GET(i->operands[0]);
}

Instruction* gen_jmp(uint16_t opcode, M68k* m)
{
    Instruction* i = calloc(1, sizeof(Instruction));
    i->context = m;
    i->name = "JMP";
    i->func = jmp;

    i->operands = calloc(1, sizeof(Operand));
    i->operands[0] = operand_make(FRAGMENT(opcode, 5, 0), i);
    i->operand_count = 1;

    return i;
}

void jsr(Instruction* i)
{
    m68k_push(i->context->pc);
    m68k_jump(GET(i->operands[0]));
}

Instruction* gen_jsr(uint16_t opcode, M68k* m)
{
    Instruction* i = calloc(1, sizeof(Instruction));
    i->context = m;
    i->name = "JSR";
    i->func = jsr;

    i->operands = calloc(1, sizeof(Operand));
    i->operands[0] = operand_make(FRAGMENT(opcode, 5, 0), i);
    i->operand_count = 1;

    return i;
}

void rts(Instruction* i)
{
    i->context->pc = m68k_pop();
}

Instruction* gen_rts(uint16_t opcode, M68k* m)
{
    Instruction* i = calloc(1, sizeof(Instruction));
    i->context = m;
    i->name = "RTS";
    i->func = rts;

    i->operands = calloc(2, sizeof(Operand));
    i->operands[0] = operand_make(FRAGMENT(opcode, 5, 0), i);
    i->operand_count = 1;

    return i;
}
