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

#define MODE_MASK(mode) (1 << mode)

#define MODES_DATA        (MODE_MASK(DataRegister))
#define MODES_ADDR        (MODE_MASK(AddressRegister))
#define MODES_ADDR_IND    (MODE_MASK(AddressRegisterIndirect) | MODE_MASK(AddressRegisterIndirectPostInc) | MODE_MASK(AddressRegisterIndirectPreDec))
#define MODES_ADDR_OFFSET (MODE_MASK(AddressRegisterIndirectDisplacement) | MODE_MASK(AddressRegisterIndirectIndexed))
#define MODES_ABS         (MODE_MASK(AbsoluteShort) | MODE_MASK(AbsoluteLong))
#define MODES_PC          (MODE_MASK(ProgramCounterDisplacement) | MODE_MASK(ProgramCounterIndexed))
#define MODES_IMM         (MODE_MASK(Immediate))
#define MODES_ALL         (MODES_DATA | MODES_ADDR | MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS | MODES_PC | MODES_IMM)
#define MODES_NONE        0

static Pattern all_patterns[] =
{
    { 0x003C, 0xFFFF, &gen_ori_ccr, MODES_IMM, MODES_NONE },
    { 0x007C, 0xFFFF, &gen_ori_sr, MODES_IMM, MODES_NONE },
    { 0x0000, 0xFF00, &gen_ori, MODES_IMM, MODES_DATA | MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS },
    { 0x023C, 0xFFFF, &gen_andi_ccr, MODES_IMM, MODES_NONE },
    { 0x027C, 0xFFFF, &gen_andi_sr, MODES_IMM, MODES_NONE },
    { 0x0200, 0xFF00, &gen_andi, MODES_IMM, MODES_DATA | MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS },
    { 0x0400, 0xFF00, &gen_subi, MODES_IMM, MODES_DATA | MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS },
    { 0x0600, 0xFF00, &gen_addi, MODES_IMM, MODES_DATA | MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS },
    { 0x0A3C, 0xFFFF, &gen_eori_ccr, MODES_IMM, MODES_NONE },
    { 0x0A7C, 0xFFFF, &gen_eori_sr, MODES_IMM, MODES_NONE },
    { 0x0A00, 0xFF00, &gen_eori, MODES_IMM, MODES_DATA | MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS },
    { 0x0C00, 0xFF00, &gen_cmpi, MODES_IMM, MODES_ALL & ~(MODES_ADDR | MODES_IMM) },
    { 0x0800, 0xFFC0, &gen_btst_imm, MODES_IMM, MODES_DATA | MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS }, // TODO subtlety with Dn/Long?
    { 0x0840, 0xFFC0, &gen_bchg_imm, MODES_IMM, MODES_DATA | MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS },
    { 0x0880, 0xFFC0, &gen_bclr_imm, MODES_IMM, MODES_DATA | MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS },
    { 0x08C0, 0xFFC0, &gen_bset_imm, MODES_IMM, MODES_DATA | MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS },
    { 0x0100, 0xF1C0, &gen_btst, MODES_DATA, MODES_DATA | MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS },
    { 0x0140, 0xF1C0, &gen_bchg, MODES_DATA, MODES_DATA | MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS },
    { 0x0180, 0xF1C0, &gen_bclr, MODES_DATA, MODES_DATA | MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS },
    { 0x01C0, 0xF1C0, &gen_bset, MODES_DATA, MODES_DATA | MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS },
    { 0x0088, 0xF0B8, &gen_movep, MODE_MASK(AddressRegisterIndirectDisplacement), MODES_DATA },
    { 0x0088, 0xF0B8, &gen_movep, MODES_DATA, MODE_MASK(AddressRegisterIndirectDisplacement) },
    { 0x0040, 0xC1C0, &gen_movea, MODES_ALL, MODES_ADDR },
    { 0x0000, 0xC000, &gen_move, MODES_ALL, MODES_DATA | MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS },
    { 0x40C0, 0xFFC0, &gen_move_from_sr, MODES_NONE, MODES_DATA | MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS },
    { 0x44C0, 0xFFC0, &gen_move_to_ccr, MODES_ALL & ~MODES_ADDR },
    { 0x46C0, 0xFFC0, &gen_move_to_sr, MODES_ALL & ~MODES_ADDR },
    { 0x4000, 0xFF00, &gen_negx, MODES_DATA | MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS, MODES_NONE },
    { 0x41C0, 0xF1C0, &gen_lea, MODE_MASK(AddressRegisterIndirect) | MODES_ADDR_OFFSET | MODES_ABS | MODES_PC, MODES_ADDR },
    { 0x4200, 0xFF00, &gen_clr, MODES_DATA | MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS, MODES_NONE },
    { 0x4400, 0xFF00, &gen_neg, MODES_DATA | MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS, MODES_NONE },
    { 0x4600, 0xFF00, &gen_not, MODES_DATA | MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS, MODES_NONE },
    { 0x4800, 0xFFC0, &gen_nbcd, MODES_DATA | MODES_ADDR | MODES_ADDR_OFFSET | MODES_ABS, MODES_NONE },
    { 0x4840, 0xFFF8, &gen_swap, MODES_DATA, MODES_NONE },
    { 0x4840, 0xFFC0, &gen_pea, MODE_MASK(AddressRegisterIndirect) | MODES_ADDR_OFFSET | MODES_ABS | MODES_PC },
    { 0x4AFC, 0xFFFF, &gen_illegal, MODES_NONE, MODES_NONE },
    { 0x4E50, 0xFFF8, &gen_link, MODES_ADDR, MODE_MASK(Immediate) },
    { 0x4E58, 0xFFF8, &gen_unlk, MODES_ADDR },
    { 0x4880, 0xFEB8, &gen_ext, MODES_DATA, MODES_NONE },
    { 0x4880, 0xFF80, &gen_movem, MODES_NONE, MODE_MASK(AddressRegisterIndirect) | MODE_MASK(AddressRegisterIndirectPreDec) | MODES_ADDR_OFFSET | MODES_ABS },
    { 0x4C80, 0xFF80, &gen_movem, MODES_ALL & ~(MODES_DATA | MODES_ADDR | MODES_IMM), MODES_NONE },
    { 0x4AC0, 0xFFC0, &gen_tas, MODES_DATA | MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS, MODES_NONE },
    { 0x4A00, 0xFF00, &gen_tst, MODES_ALL & ~(MODES_ADDR | MODES_IMM), MODES_NONE },
    { 0x4E40, 0xFFF0, &gen_trap, MODE_MASK(Value), MODES_NONE },
    { 0x4E60, 0xFFF8, &gen_move_usp, MODES_ADDR, MODES_NONE }, //
    { 0x4E68, 0xFFF8, &gen_move_usp, MODES_NONE, MODES_ADDR }, //
    { 0x4E70, 0xFFFF, &gen_reset, MODES_NONE, MODES_NONE },
    { 0x4E71, 0xFFFF, &gen_nop, MODES_NONE, MODES_NONE },
    { 0x4E72, 0xFFFF, &gen_stop, MODES_NONE, MODES_NONE }, //
    { 0x4E73, 0xFFFF, &gen_rte, MODES_NONE, MODES_NONE },
    { 0x4E75, 0xFFFF, &gen_rts, MODES_NONE, MODES_NONE },
    { 0x4E76, 0xFFFF, &gen_trapv, MODES_NONE, MODES_NONE },
    { 0x4E77, 0xFFFF, &gen_rtr, MODES_NONE, MODES_NONE },
    { 0x4E80, 0xFFC0, &gen_jsr, MODE_MASK(AddressRegisterIndirect) | MODES_ADDR_OFFSET | MODES_ABS | MODES_PC, MODES_NONE },
    { 0x4EC0, 0xFFC0, &gen_jmp, MODE_MASK(AddressRegisterIndirect) | MODES_ADDR_OFFSET | MODES_ABS | MODES_PC, MODES_NONE },
    { 0x4180, 0xF1C0, &gen_chk, MODES_ALL & ~MODES_ADDR, MODES_DATA },
    { 0x50C8, 0xF0F8, &gen_dbcc, MODES_DATA, MODE_MASK(BranchingOffset) },
    { 0x5000, 0xF100, &gen_addq, MODE_MASK(Value), MODES_ALL & ~(MODES_PC | MODES_IMM) },
    { 0x5100, 0xF100, &gen_subq, MODE_MASK(Value), MODES_ALL & ~(MODES_PC | MODES_IMM) },
    { 0x50C0, 0xF0C0, &gen_scc, MODES_DATA | MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS, MODES_NONE },
    { 0x6000, 0xFF00, &gen_bra, MODE_MASK(BranchingOffset), MODES_NONE },
    { 0x6100, 0xFF00, &gen_bsr, MODE_MASK(BranchingOffset), MODES_NONE },
    { 0x6000, 0xF000, &gen_bcc, MODE_MASK(BranchingOffset), MODES_NONE },
    { 0x7000, 0xF100, &gen_moveq, MODE_MASK(Value), MODES_DATA },
    { 0x80C0, 0xF1C0, &gen_divu, MODES_ALL & ~MODES_ADDR, MODES_DATA },
    { 0x81C0, 0xF1C0, &gen_divs, MODES_ALL & ~MODES_ADDR, MODES_DATA },
    { 0x8100, 0xF1F8, &gen_sbcd, MODES_DATA, MODES_DATA },
    { 0x8108, 0xF1F8, &gen_sbcd, MODE_MASK(AddressRegisterIndirectPreDec), MODE_MASK(AddressRegisterIndirectPreDec) },
    { 0x8000, 0xF100, &gen_or, MODES_ALL & ~MODES_ADDR, MODES_DATA },
    { 0x8100, 0xF100, &gen_or, MODES_DATA, MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS },
    { 0x9000, 0xF100, &gen_sub, MODES_ALL, MODES_DATA },
    { 0x9100, 0xF100, &gen_sub, MODES_DATA, MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS },
    { 0x9100, 0xF138, &gen_subx, MODES_DATA, MODES_DATA },
    { 0x9108, 0xF138, &gen_subx, MODE_MASK(AddressRegisterIndirectPreDec), MODE_MASK(AddressRegisterIndirectPreDec) },
    { 0x90C0, 0xF0C0, &gen_suba, MODES_ALL, MODES_ADDR },
    { 0xB100, 0xF100, &gen_eor, MODES_DATA, MODES_DATA | MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS },
    { 0xB108, 0xF138, &gen_cmpm, MODE_MASK(AddressRegisterIndirectPostInc), MODE_MASK(AddressRegisterIndirectPostInc) },
    { 0xB000, 0xF100, &gen_cmp, MODES_ALL, MODES_DATA },
    { 0xB0C0, 0xF0C0, &gen_cmpa, MODES_ALL, MODES_ADDR },
    { 0xC0C0, 0xF1C0, &gen_mulu, MODES_ALL & ~MODES_ADDR, MODES_DATA },
    { 0xC1C0, 0xF1C0, &gen_muls, MODES_ALL & ~MODES_ADDR, MODES_DATA },
    { 0xC100, 0xF1F8, &gen_abcd, MODES_DATA, MODES_DATA },
    { 0xC108, 0xF1F8, &gen_abcd, MODE_MASK(AddressRegisterIndirectPreDec), MODE_MASK(AddressRegisterIndirectPreDec) },
    { 0xC000, 0xF100, &gen_and, MODES_ALL & ~MODES_ADDR, MODES_DATA },
    { 0xC100, 0xF100, &gen_and, MODES_DATA, MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS },
    { 0xD100, 0xF138, &gen_addx, MODES_DATA, MODES_DATA },
    { 0xD108, 0xF138, &gen_addx, MODE_MASK(AddressRegisterIndirectPreDec), MODE_MASK(AddressRegisterIndirectPreDec) },
    { 0xD0C0, 0xF0C0, &gen_adda, MODES_ALL, MODES_ADDR },
    { 0xD000, 0xF100, &gen_add, MODES_ALL, MODES_DATA },
    { 0xD100, 0xF100, &gen_add, MODES_DATA, MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS },
    { 0xC140, 0xF1F8, &gen_exg, MODES_DATA, MODES_DATA },
    { 0xC148, 0xF1F8, &gen_exg, MODES_ADDR, MODES_ADDR },
    { 0xC188, 0xF1F8, &gen_exg, MODES_DATA, MODES_ADDR },
    { 0xE0C0, 0xFEC0, &gen_asd_mem, MODES_NONE, MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS },
    { 0xE2C0, 0xFEC0, &gen_lsd_mem, MODES_NONE, MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS },
    { 0xE4C0, 0xFEC0, &gen_roxd_mem, MODES_NONE, MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS },
    { 0xE6C0, 0xFEC0, &gen_rod_mem, MODES_NONE, MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS },
    { 0xE000, 0xF020, &gen_asd, MODE_MASK(Value), MODES_DATA },
    { 0xE020, 0xF020, &gen_asd, MODES_DATA, MODES_DATA },
    { 0xE008, 0xF038, &gen_lsd, MODE_MASK(Value), MODES_DATA },
    { 0xE028, 0xF038, &gen_lsd, MODES_DATA, MODES_DATA },
    { 0xE010, 0xF038, &gen_roxd, MODE_MASK(Value), MODES_DATA },
    { 0xE030, 0xF038, &gen_roxd, MODES_DATA, MODES_DATA },
    { 0xE018, 0xF038, &gen_rod, MODE_MASK(Value), MODES_DATA },
    { 0xE038, 0xF038, &gen_rod, MODES_DATA, MODES_DATA }
};

#define PATTERN_MATCH(OPCODE, PATTERN) ((OPCODE & PATTERN.mask) == PATTERN.pattern)
#define PATTERN_GENERATE(PATTERN, OPCODE, CONTEXT) PATTERN.generator(OPCODE, CONTEXT)

// TODO decouple m68k instance and instructions

bool instruction_is_valid2(Instruction* i, Pattern* pattern)
{
    if (i == NULL || i->size == InvalidSize)
        return false;

    if (i->src == NULL && pattern->src_addressing_modes != MODES_NONE ||              // Missing operand
        i->src != NULL && !(MODE_MASK(i->src->type) & pattern->src_addressing_modes)) // Illegal operand
        return false;

    if (i->dst == NULL && pattern->dst_addressing_modes != MODES_NONE ||
        i->dst != NULL && !(MODE_MASK(i->dst->type) & pattern->dst_addressing_modes))
        return false;

    return true;
}

Instruction* instruction_generate(M68k* context, uint16_t opcode)
{
    // Given an opcode, this function returns the corresponding instruction.
    //
    // Strategy:
    //   1. Loop through the pattern list until a match is found
    //   2. Decode the operands and size from the opcode to generate the instruction 
    //   3. Check that the instruction is valid (legal addressing modes, valid size)
    //      - yes: Return the instruction
    //      - no:  Discard the instruction and continue looking for a match

    uint8_t pattern_count = sizeof(all_patterns) / sizeof(Pattern);

    for (uint8_t i = 0; i < pattern_count; ++i)
    {
        if (PATTERN_MATCH(opcode, all_patterns[i]))
        {
            if (opcode == 0x18b)
                printf("a");

            // Generate the instruction
            Instruction* instr = PATTERN_GENERATE(all_patterns[i], opcode, context);

            // If the generated instruction is invalid, continue looking for matches
            if (!instruction_is_valid2(instr, &all_patterns[i]))
            {
                instruction_free(instr);
                continue;
            }

            instr->opcode = opcode; // TODO in instruction_make?
            return instr;
        }
    }

    return NULL;
}

bool instruction_is_valid(Instruction* instr, bool must_have_src, bool must_have_dst)
{
    if (instr == NULL || instr->size == InvalidSize)
        return false;

    if (instr->src != NULL && instr->src->type == Unsupported ||
        instr->src == NULL && must_have_src)
        return false;

    if (instr->dst != NULL && instr->dst->type == Unsupported ||
        instr->dst == NULL && must_have_dst)
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
