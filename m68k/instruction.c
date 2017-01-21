#include <stdlib.h>

#include "instruction.h"
#include "m68k.h"
#include "operands.h"

Instruction* instruction_make(M68k* context, char* name, InstructionFunc func)
{
    Instruction* i = calloc(1, sizeof(Instruction));
    i->context = context;
    i->name = name;
    i->func = func;
    return i;
}

void instruction_free(Instruction* instr)
{
    if (instr == NULL)
        return;

    free(instr->src);
    free(instr->dst);
    free(instr);
}

static Pattern all_patterns[] =
{
    { 0x0200, 0xFF00, &gen_andi },
    { 0x0100, 0xF1C0, &gen_btst }, // TODO other btst form
    { 0x0140, 0xF1C0, &gen_bchg }, // TODO other bchg form
    { 0x0180, 0xF1C0, &gen_bclr }, // TODO other bclr form
    { 0x01C0, 0xF1C0, &gen_bset }, // TODO other bset form
    { 0x0040, 0xC1C0, &gen_movea },
    { 0x0000, 0xC000, &gen_move },
    { 0x41C0, 0xF1C0, &gen_lea },
    { 0x4200, 0xFF00, &gen_clr },
    { 0x4600, 0xFF00, &gen_not },
    { 0x4840, 0xFFF8, &gen_swap },
    { 0x4840, 0xFFC0, &gen_pea },
    { 0x4880, 0xFEB8, &gen_ext },
    { 0x4880, 0xFB80, &gen_movem },
    { 0x4A00, 0xFF00, &gen_tst },
    { 0x4E60, 0xFFF0, &gen_move_usp },
    { 0x4E75, 0xFFFF, &gen_rts },
    { 0x4E80, 0xFFC0, &gen_jsr },
    { 0x4EC0, 0xFFC0, &gen_jmp },
    //{ 0x5000, 0xF000, &gen_scc },
    { 0x50C8, 0xF0F8, &gen_dbcc },
    { 0x6000, 0xFF00, &gen_bra },
    { 0x6100, 0xFF00, &gen_bsr },
    { 0x6000, 0xF000, &gen_bcc },
    { 0x7000, 0xF100, &gen_moveq },
    { 0x8000, 0xF000, &gen_or },
    { 0xB000, 0xF000, &gen_eor },
    { 0xC100, 0xF100, &gen_exg },
    { 0xC000, 0xF000, &gen_and },
    { 0xD000, 0xF000, &gen_add },
    { 0xC0C0, 0xF1C0, &gen_mulu },
    { 0xC1C0, 0xF1C0, &gen_muls },
    { 0xE2C0, 0xFEC0, &gen_lsX },
};

int pattern_match(uint16_t opcode, Pattern pattern)
{
    return (opcode & pattern.mask) == pattern.pattern;
}

Instruction* pattern_generate(Pattern pattern, uint16_t opcode, M68k* context)
{
    return pattern.generator(opcode, context);
}

Instruction* instruction_generate(M68k* context, uint16_t opcode)
{
    int pattern_count = sizeof(all_patterns) / sizeof(Pattern);

    for (int i = 0; i < pattern_count; ++i)
        if (pattern_match(opcode, all_patterns[i]))
        {
            // Generate the instruction
            Instruction* instr = pattern_generate(all_patterns[i], opcode, context);

            if (instr == NULL)
                return NULL;

            // Compute its length in bytes
            instr->length += 2 + operand_length(instr->src) + operand_length(instr->dst);

            return instr;
        }

    return NULL;
}

bool instruction_valid(Instruction* instr)
{
    // Check that the generated instruction is valid
    if (instr == NULL)
        return false;

    return true;
}

void instruction_execute(Instruction* instr)
{
    // Pre-execution actions
    if (instr->src != NULL && instr->src->pre != NULL)
        instr->src->pre(instr->src);
    if (instr->dst != NULL && instr->dst->pre != NULL)
        instr->dst->pre(instr->dst);

    instr->func(instr);

    // Post-execution actions
    if (instr->src != NULL && instr->src->post != NULL)
        instr->src->post(instr->src);
    if (instr->dst != NULL && instr->dst->post != NULL)
        instr->dst->post(instr->dst);
}
