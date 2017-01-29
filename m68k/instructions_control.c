#include <stdbool.h>
#include <stdlib.h>

#include "bit_utils.h"
#include "conditions.h"
#include "instruction.h"
#include "m68k.h"
#include "operands.h"

int bcc(Instruction* i)
{
    if (i->condition->func(i->context))
    {
        uint16_t displacement = GET(i->src); // TODO does this always work? not if 8bit offset stored in 16bit part for soem reason
        i->context->pc += displacement > MAX_VALUE(Byte) ? (int16_t) displacement : (int8_t) displacement;

        return 0; // TODO timings
    }

    return 0;
}

Instruction* gen_bcc(uint16_t opcode, M68k* m) // TODO factor with bra?
{
    Instruction* i = instruction_make(m, "BCC", bcc);
    // TODO set name wrt condition

    i->condition = condition_get(FRAGMENT(opcode, 11, 8));
    //sprintf(i->name, "B%s", i->condition->mnemonics);

    int displacement = FRAGMENT(opcode, 7, 0);
    if (displacement == 0)
        i->src = operand_make_branching_offset(i, Word);
    else if (displacement == 0xFF)
        i->src = operand_make_branching_offset(i, Long); // Seems to be supported by 68020+ only
    else
        i->src = operand_make_branching_offset(i, Byte);

    return i;
}

int bra(Instruction* i)
{
    int32_t displacement = m68k_read_b(i->context, i->context->pc + 1);
    if (displacement == 0)
        displacement = m68k_read_w(i->context, i->context->pc + 2);
    else if (displacement == 0xFF)
        displacement = m68k_read_l(i->context, i->context->pc + 2);

    i->context->pc += displacement;

    return 0;
}

Instruction* gen_bra(uint16_t opcode, M68k* m)
{
    return instruction_make(m, "BRA", bra);;
}

int bsr(Instruction* i)
{
    m68k_push(i->context->pc);
    m68k_jump(i->context->pc + GET(i->src));

    return 0;
}

Instruction* gen_bsr(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "BSR", bsr);

    /*int displacement = FRAGMENT(opcode, 7, 0);
    if (displacement == 0)
        i->src = operand_make_absolute_short(16, i);
    else if (displacement == 0xFF)
        i->src = operand_make_absolute_short(32, i);
    else
        i->src = operand_make_immediate(FRAGMENT(opcode, 7, 0), i);*/

    return i;
}

int dbcc(Instruction* i)
{
    //if (i->condition->func(i->context)) TODO unclear about the condition

    uint16_t reg = GET(i->src);
    if (reg > 0)
        i->context->pc += (int16_t)m68k_read_w(i->context, i->context->pc + 2) - 2;

    SET(i->src, reg - 1);

    return 0; // TODO timings
}

Instruction* gen_dbcc(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "DBcc", dbcc);// TODO set name wrt condition
    i->size = Word;
    i->base_length = 4;
    i->condition = condition_get(FRAGMENT(opcode, 11, 8));
    //sprintf(i->name, "B%s", i->condition->mnemonics);
    i->src = operand_make_data_register(FRAGMENT(opcode, 2, 0), i);
    return i;
}

int jmp(Instruction* i)
{
    i->context->pc = GET(i->src);

    return 0;
}

Instruction* gen_jmp(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "JMP", jmp);
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);
    return i;
}

int jsr(Instruction* i)
{
    m68k_push(i->context->pc);
    m68k_jump(GET(i->src));

    return 0;
}

Instruction* gen_jsr(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "JSR", jsr);
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);
    return i;
}

int rts(Instruction* i)
{
    i->context->pc = m68k_pop();

    return 0;
}

Instruction* gen_rts(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "RTS", rts);
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);
    return i;
}
