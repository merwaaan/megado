#include <stdbool.h>
#include <stdlib.h>

#include "bit_utils.h"
#include "conditions.h"
#include "instruction.h"
#include "m68k.h"
#include "operands.h"

int bcc(Instruction* i)
{
    uint32_t offset = i->context->instruction_register & 0xFF;
    if (offset == 0)
        offset = m68k_fetch(i->context);
    else if (offset == 0xFF)
        offset = m68k_fetch(i->context) << 16 | m68k_fetch(i->context);

    if (i->condition->func(i->context))
    {
        i->context->pc += offset > MAX_VALUE(Byte) ? (int16_t)offset : (int8_t)offset;

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

    /*int displacement = FRAGMENT(opcode, 7, 0);
    if (displacement == 0)
        i->src = operand_make_branching_offset(i, Word);
    else if (displacement == 0xFF)
        i->src = operand_make_branching_offset(i, Long); // 68020+ only?
    else
        i->src = operand_make_branching_offset(i, Byte);*/

    return i;
}

int bra(Instruction* i)
{
    uint8_t displacement = i->context->instruction_register & 0xFF;

    if (displacement == 0) // TODO step
        i->context->pc += (int16_t)m68k_read_w(i->context, i->context->pc + 2);
    else if (displacement == 0xFF) // TODO step
        i->context->pc += (int32_t)m68k_read_l(i->context, i->context->pc + 2);
    else
        i->context->pc += (int8_t)displacement;

    return 0;
}

Instruction* gen_bra(uint16_t opcode, M68k* m)
{
    return instruction_make(m, "BRA", bra); // TODO
}

int bsr(Instruction* i)
{
    uint32_t offset = i->context->instruction_register & 0xFF;
    if (offset == 0)
        offset = m68k_fetch(i->context);
    else if (offset == 0xFF)
        offset = m68k_fetch(i->context) << 16 | m68k_fetch(i->context);

    i->context->address_registers[7] -= 4;
    m68k_write_l(i->context, i->context->address_registers[7], i->context->pc);

    i->context->pc += offset - 2;

    return 0;
}

Instruction* gen_bsr(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "BSR", bsr);

    /*int displacement = FRAGMENT(opcode, 7, 0);
    if (displacement == 0)
        i->src = operand_make_branching_offset(i, Word);
    else if (displacement == 0xFF)
        i->src = operand_make_branching_offset(i, Long); // 68020+ only?
    else
        i->src = operand_make_branching_offset(i, Byte);*/

    return i;
}

int dbcc(Instruction* i)
{
    //if (i->condition->func(i->context)) TODO unclear about the condition

    int16_t offset = m68k_fetch(i->context); // TODO should only be read if cond true??

    uint16_t reg = GETE(i->src);
    if (reg > 0)
        i->context->pc += offset - 2;

    SETE(i->src, reg - 1);

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
    // TODO add offset as operand
    return i;
}

int jmp(Instruction* i)
{
    i->context->pc = FETCH_EA_AND_GETE(i->src);

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
    uint32_t ea = FETCH_EAE(i->dst) - 2; // TODO always -2?

    // Push the address following the instruction to the stack
    i->context->address_registers[7] -= 4;
    m68k_write_l(i->context, i->context->address_registers[7], i->context->pc);

    // Jump to the location specified by ea
    i->context->pc = ea;

    return 0;
}

Instruction* gen_jsr(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "JSR", jsr);
    i->dst = operand_make(FRAGMENT(opcode, 5, 0), i);
    return i;
}

int nop(Instruction* i)
{
    return 0;
}

Instruction* gen_nop(uint16_t opcode, M68k* m)
{
    return instruction_make(m, "NOP", nop);
}

int rts(Instruction* i)
{
    //i->context->pc = m68k_pop();

    i->context->pc = m68k_read_l(i->context, i->context->address_registers[7]);
    i->context->address_registers[7] += 4;

    return 0;
}

Instruction* gen_rts(uint16_t opcode, M68k* m)
{
    Instruction* i = instruction_make(m, "RTS", rts);
    i->src = operand_make(FRAGMENT(opcode, 5, 0), i);
    return i;
}
