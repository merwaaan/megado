#include <stdlib.h>
#include <string.h>

#include "instruction.h"
#include "m68k.h"
#include "operands.h"

int not_implemented(Instruction* i)
{
    return 0;
}

Instruction* instruction_make(M68k* context, char* name, InstructionFunc func)
{
    Instruction* i = calloc(1, sizeof(Instruction));
    i->context = context;
    i->func = func;
    i->base_cycles = 0; // TODO

    // Dynamically allocate memory for the instruction name
    // and copy the name argument whatever its source in order
    // to avoid discrepancies between string literals and
    // dynamic strings.
    i->name = calloc(20, sizeof(char));
    strcpy(i->name, name);

    return i;
}

void instruction_free(Instruction* instr)
{
    if (instr == NULL)
        return;

    free(instr->name);
    free(instr->src);
    free(instr->dst);
    free(instr);
}

static Pattern all_patterns[] =
{
    { 0x003C, 0xFFFF, &gen_ori_ccr },
    { 0x007C, 0xFFFF, &gen_ori_sr },
    { 0x0000, 0xFF00, &gen_ori },
    { 0x023C, 0xFFFF, &gen_andi_ccr },
    { 0x027C, 0xFFFF, &gen_andi_sr },
    { 0x0200, 0xFF00, &gen_andi },
    { 0x0400, 0xFF00, &gen_subi },
    { 0x0600, 0xFF00, &gen_addi },
    { 0x0A3C, 0xFFFF, &gen_eori_ccr },
    { 0x0A7C, 0xFFFF, &gen_eori_sr },
    { 0x0A00, 0xFF00, &gen_eori },
    { 0x0C00, 0xFF00, &gen_cmpi },
    { 0x0800, 0xFFC0, &gen_btst_imm },
    { 0x0840, 0xFFC0, &gen_bchg_imm },
    { 0x0880, 0xFFC0, &gen_bclr_imm },
    { 0x08C0, 0xFFC0, &gen_bset_imm },
    { 0x0100, 0xF1C0, &gen_btst },
    { 0x0140, 0xF1C0, &gen_bchg },
    { 0x0180, 0xF1C0, &gen_bclr },
    { 0x01C0, 0xF1C0, &gen_bset },
    { 0x0108, 0xF138, &gen_movep },
    { 0x0040, 0xC1C0, &gen_movea },
    { 0x0000, 0xC000, &gen_move },
    { 0x40C0, 0xFFC0, &gen_move_from_sr },
    { 0x44C0, 0xFFC0, &gen_move_to_ccr },
    { 0x46C0, 0xFFC0, &gen_move_to_sr },
    { 0x4000, 0xFF00, &gen_negx },
    { 0x41C0, 0xF1C0, &gen_lea },
    { 0x4200, 0xFF00, &gen_clr },
    { 0x4400, 0xFF00, &gen_neg },
    { 0x4600, 0xFF00, &gen_not },
    { 0x4800, 0xFFC0, &gen_nbcd },
    { 0x4840, 0xFFF8, &gen_swap },
    { 0x4840, 0xFFC0, &gen_pea },
    { 0x4AFC, 0xFFFF, &gen_illegal },
    { 0x4E50, 0xFFF8, &gen_link },
    { 0x4E58, 0xFFF8, &gen_unlk },
    { 0x4880, 0xFEB8, &gen_ext },
    { 0x4880, 0xFB80, &gen_movem },
    { 0x4AC0, 0xFFC0, &gen_tas },
    { 0x4A00, 0xFF00, &gen_tst },
    { 0x4E40, 0xFFF0, &gen_trap },
    { 0x4E60, 0xFFF0, &gen_move_usp },
    { 0x4E70, 0xFFFF, &gen_reset },
    { 0x4E71, 0xFFFF, &gen_nop },
    { 0x4E72, 0xFFFF, &gen_stop },
    { 0x4E73, 0xFFFF, &gen_rte },
    { 0x4E75, 0xFFFF, &gen_rts },
    { 0x4E76, 0xFFFF, &gen_trapv },
    { 0x4E77, 0xFFFF, &gen_rtr },
    { 0x4E80, 0xFFC0, &gen_jsr },
    { 0x4EC0, 0xFFC0, &gen_jmp },
    { 0x4180, 0xF1C0, &gen_chk },
    { 0x50C8, 0xF0F8, &gen_dbcc },
    { 0x5000, 0xF100, &gen_addq },
    { 0x5100, 0xF100, &gen_subq },
    { 0x50C0, 0xF0C0, &gen_scc },
    { 0x6000, 0xFF00, &gen_bra },
    { 0x6100, 0xFF00, &gen_bsr },
    { 0x6000, 0xF000, &gen_bcc },
    { 0x7000, 0xF100, &gen_moveq },
    { 0x80C0, 0xF1C0, &gen_divu },
    { 0x81C0, 0xF1C0, &gen_divs },
    { 0x8100, 0xF1F0, &gen_sbcd },
    { 0x8000, 0xF000, &gen_or },
    { 0x9000, 0xF000, &gen_sub },
    { 0x9100, 0xF000, &gen_subx },
    { 0x90C0, 0xF0C0, &gen_suba },
    { 0xB100, 0xF100, &gen_eor },
    { 0xB108, 0xF138, &gen_cmpm },
    { 0xB000, 0xF100, &gen_cmp },
    { 0xB0C0, 0xF0C0, &gen_cmpa },
    { 0xC0C0, 0xF1C0, &gen_mulu },
    { 0xC1C0, 0xF1C0, &gen_muls },
    { 0xC100, 0xF1F0, &gen_abcd },
    { 0xC000, 0xF000, &gen_and },
    { 0xD100, 0xF130, &gen_addx },
    { 0xD0C0, 0xF0C0, &gen_adda },
    { 0xD000, 0xF000, &gen_add },
    { 0xC100, 0xF100, &gen_exg },
    { 0xE0C0, 0xFEC0, &gen_asd_mem },
    { 0xE2C0, 0xFEC0, &gen_lsd_mem },
    { 0xE4C0, 0xFEC0, &gen_roxd_mem },
    { 0xE6C0, 0xFEC0, &gen_rod_mem },
    { 0xE000, 0xF018, &gen_asd },
    { 0xE008, 0xF018, &gen_lsd },
    { 0xE010, 0xF018, &gen_roxd },
    { 0xE018, 0xF018, &gen_rod },
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

            // If the generated instruction is invalid, try the following opcodes
            if (instr == NULL ||
                instr->size == InvalidSize ||
                (instr->src != NULL && instr->src->type == Unsupported) ||
                (instr->dst != NULL && instr->dst->type == Unsupported))
            {
                free(instr);
                continue;
            }

            return instr;
        }

    return NULL;
}

bool instruction_is_valid(Instruction* instr, bool has_src, bool has_dst)
{
    if (instr == NULL || instr->size == InvalidSize)
        return false;

    if (has_src && (instr->src == NULL || instr->src->type == Unsupported))
        return false;

    if (has_dst && (instr->dst == NULL || instr->dst->type == Unsupported))
        return false;

    return true;
}

int instruction_execute(Instruction* instr)
{
    // TODO faster to use noops?
    // Pre-execution actions
    if (instr->src != NULL && instr->src->pre_func != NULL)
        instr->src->pre_func(instr->src);
    if (instr->dst != NULL && instr->dst->pre_func != NULL)
        instr->dst->pre_func(instr->dst);

    int additional_cycles = instr->func(instr);

    // Post-execution actions
    if (instr->src != NULL && instr->src->post_func != NULL)
        instr->src->post_func(instr->src);
    if (instr->dst != NULL && instr->dst->post_func != NULL)
        instr->dst->post_func(instr->dst);

    return instr->base_cycles + additional_cycles;
}


void decoded_instruction_free(DecodedInstruction* decoded)
{
    if (decoded == NULL)
        return;

    free(decoded->mnemonics);
    free(decoded);
}
