#include <stdbool.h>
#include <stdlib.h>

#include "bit_utils.h"
#include "conditions.h"
#include "instruction.h"
#include "m68k.h"
#include "operands.h"

void bcc(Instruction* i)
{
    if (i->condition->func(i->context))
        i->context->pc += GET(i->src);
}

Instruction* gen_bcc(uint16_t opcode, M68k* m) // TODO factor with bra?
{
    Instruction* i = calloc(1, sizeof(Instruction));
    i->context = m;
    i->func = bcc;
    i->name = "BCC"; // TODO set name wrt condition

    i->condition = condition_get(FRAGMENT(opcode, 11, 8));
    //sprintf(i->name, "B%s", i->condition->mnemonics);

    int displacement = FRAGMENT(opcode, 7, 0);
    if (displacement == 0)
        i->src = operand_make_branching_offset(i, Word);
    else if (displacement == 0xFF)
        i->src = operand_make_branching_offset(i, Long);
    else
        i->src = operand_make_branching_offset(i, Byte);

    return i;
}

void bra(Instruction* i)
{
    m68k_jump(i->context->pc + GET(i->src));
}

Instruction* gen_bra(uint16_t opcode, M68k* m)
{
    Instruction* i = calloc(1, sizeof(Instruction));
    i->context = m;
    i->name = "BRA";
    i->func = bra;

    /*int displacement = FRAGMENT(opcode, 7, 0);
    if (displacement == 0)
        i->src = operand_make_absolute_short(16, i);
    else if (displacement == 0xFF)
        i->src = operand_make_absolute_short(32, i);
    else
        i->src = operand_make_immediate(FRAGMENT(opcode, 7, 0), i);*/

    return i;
}

void bsr(Instruction* i)
{
    m68k_push(i->context->pc);
    m68k_jump(i->context->pc + GET(i->src));
}

Instruction* gen_bsr(uint16_t opcode, M68k* m)
{
    Instruction* i = calloc(1, sizeof(Instruction));
    i->context = m;
    i->name = "BSR";
    i->func = bsr;

    /*int displacement = FRAGMENT(opcode, 7, 0);
    if (displacement == 0)
        i->src = operand_make_absolute_short(16, i);
    else if (displacement == 0xFF)
        i->src = operand_make_absolute_short(32, i);
    else
        i->src = operand_make_immediate(FRAGMENT(opcode, 7, 0), i);*/

    return i;
}

void dbcc(Instruction* i)
{
    //if (i->condition->func(i->context)) TODO unclear about the condition

    uint16_t reg = GET(i->src);
    if (reg > 0)
        i->context->pc += (int16_t)m68k_read_w(i->context, i->context->pc + 2) - 2;

    SET(i->src, reg - 1);
}

Instruction* gen_dbcc(uint16_t opcode, M68k* m)
{
    Instruction* i = calloc(1, sizeof(Instruction));
    i->context = m;
    i->func = dbcc;
    i->name = "DBCC"; // TODO set name wrt condition
    i->size = Word;
    i->length += 2;
    i->condition = condition_get(FRAGMENT(opcode, 11, 8));
    //sprintf(i->name, "B%s", i->condition->mnemonics);
    i->src = operand_make_data_register(FRAGMENT(opcode, 2, 0), i);
    return i;
}

void jmp(Instruction* i)
{
    i->context->pc = GET(i->src);
}

Instruction* gen_jmp(uint16_t opcode, M68k* m)
{
    Instruction* i = calloc(1, sizeof(Instruction));
    i->context = m;
    i->name = "JMP";
    i->func = jmp;
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);
    return i;
}

void jsr(Instruction* i)
{
    m68k_push(i->context->pc);
    m68k_jump(GET(i->src));
}

Instruction* gen_jsr(uint16_t opcode, M68k* m)
{
    Instruction* i = calloc(1, sizeof(Instruction));
    i->context = m;
    i->name = "JSR";
    i->func = jsr;
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);
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
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);
    return i;
}
