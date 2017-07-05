#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "instruction.h"
#include "m68k.h"
#include "operands.h"

#define DECLARE_INSTR(name) Instruction* gen_ ## name(uint16_t opcode)

// Bit-wise operations
DECLARE_INSTR(bchg);
DECLARE_INSTR(bchg_imm);
DECLARE_INSTR(bclr);
DECLARE_INSTR(bclr_imm);
DECLARE_INSTR(bset);
DECLARE_INSTR(bset_imm);
DECLARE_INSTR(btst);
DECLARE_INSTR(btst_imm);

// Logic operations
DECLARE_INSTR(and);
DECLARE_INSTR(andi);
DECLARE_INSTR(andi_ccr);
DECLARE_INSTR(andi);
DECLARE_INSTR(eor);
DECLARE_INSTR(eori);
DECLARE_INSTR(eori_ccr);
DECLARE_INSTR(or );
DECLARE_INSTR(ori);
DECLARE_INSTR(ori_ccr);
DECLARE_INSTR(not);
DECLARE_INSTR(scc);
DECLARE_INSTR(tst);

// Arithmetic operations
DECLARE_INSTR(add);
DECLARE_INSTR(adda);
DECLARE_INSTR(addi);
DECLARE_INSTR(addq);
DECLARE_INSTR(addx);
DECLARE_INSTR(clr);
DECLARE_INSTR(cmp);
DECLARE_INSTR(cmpa);
DECLARE_INSTR(cmpi);
DECLARE_INSTR(cmpm);
DECLARE_INSTR(divs);
DECLARE_INSTR(divu);
DECLARE_INSTR(ext);
DECLARE_INSTR(muls);
DECLARE_INSTR(mulu);
DECLARE_INSTR(neg);
DECLARE_INSTR(negx);
DECLARE_INSTR(sub);
DECLARE_INSTR(suba);
DECLARE_INSTR(subi);
DECLARE_INSTR(subq);
DECLARE_INSTR(subx);
DECLARE_INSTR(tas);

// Shit & rotate
DECLARE_INSTR(asd);
DECLARE_INSTR(asd_mem);
DECLARE_INSTR(lsd);
DECLARE_INSTR(lsd_mem);
DECLARE_INSTR(rod);
DECLARE_INSTR(rod_mem);
DECLARE_INSTR(roxd);
DECLARE_INSTR(roxd_mem);
DECLARE_INSTR(swap);

// Data transfer
DECLARE_INSTR(exg);
DECLARE_INSTR(lea);
DECLARE_INSTR(link);
DECLARE_INSTR(move);
DECLARE_INSTR(movea);
DECLARE_INSTR(movem);
DECLARE_INSTR(moveq);
DECLARE_INSTR(movep);
DECLARE_INSTR(move_to_ccr);
DECLARE_INSTR(pea);
DECLARE_INSTR(trap);
DECLARE_INSTR(unlk);

// Program control
DECLARE_INSTR(bcc);
DECLARE_INSTR(bra);
DECLARE_INSTR(bsr);
DECLARE_INSTR(dbcc);
DECLARE_INSTR(jmp);
DECLARE_INSTR(jsr);
DECLARE_INSTR(nop);
DECLARE_INSTR(rtd);
DECLARE_INSTR(rtr);
DECLARE_INSTR(rts);

// Binary-coded decimals
DECLARE_INSTR(abcd);
DECLARE_INSTR(nbcd);
DECLARE_INSTR(sbcd);

// Exceptions
DECLARE_INSTR(chk);
DECLARE_INSTR(illegal);
DECLARE_INSTR(trap);
DECLARE_INSTR(trapv);

// Privileged instructions
DECLARE_INSTR(andi_sr);
DECLARE_INSTR(eori_sr);
DECLARE_INSTR(ori_sr);
DECLARE_INSTR(move_from_sr);
DECLARE_INSTR(move_to_sr);
DECLARE_INSTR(move_usp);
DECLARE_INSTR(reset);
DECLARE_INSTR(rte);
DECLARE_INSTR(stop);

uint8_t not_implemented(Instruction* i, M68k* ctx)
{
    return 0;
}

// TODO move from ccr?

// Metadata to generate the M68000's instruction set.
// http://goldencrystal.free.fr/M68kOpcodes-v2.3.pdf
// https://emu-docs.org/CPU%2068k/68kstat.txt

typedef struct Pattern
{
    // Unique bit pattern of the instruction
    //
    // May contain the following tokens:
    //   - ?: any bit
    //   - 0/1: cleared/set bit
    //   - S2/S3: size
    //   - MMMXXX/XXXMMM: addressing mode
    char* bits;

    // Function that returns an instance of the instruction
    // setup with appropriate operands depending on the opcode.
    GenFunc* generator;

    // Masks describing legal modes for effective addresses
    uint16_t legal_ea_modes;
    uint16_t legal_ea_modes2; // Some instructions use two effective addresses
} Pattern;

#define MODE_MASK(mode) (1 << mode)

// Group modes that often go together
#define MODES_DATA        (MODE_MASK(DataRegister))
#define MODES_ADDR        (MODE_MASK(AddressRegister))
#define MODES_ADDR_IND    (MODE_MASK(AddressRegisterIndirect) | MODE_MASK(AddressRegisterIndirectPostInc) | MODE_MASK(AddressRegisterIndirectPreDec))
#define MODES_ADDR_OFFSET (MODE_MASK(AddressRegisterIndirectDisplacement) | MODE_MASK(AddressRegisterIndirectIndexed))
#define MODES_ABS         (MODE_MASK(AbsoluteShort) | MODE_MASK(AbsoluteLong))
#define MODES_PC          (MODE_MASK(ProgramCounterDisplacement) | MODE_MASK(ProgramCounterIndexed))
#define MODES_IMM         (MODE_MASK(Immediate))
#define MODES_ALL         (MODES_DATA | MODES_ADDR | MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS | MODES_PC | MODES_IMM)
#define MODES_NONE        0

// Note: some instructions have different legal addressing modes depending
// on the size of the operation (eg. the address register is often not
// allowed for byte-size operation). In this case, we HAVE to spell out
// the legal modes for each size instead of using the S2/S3 tokens in the
// opcode pattern.

static Pattern instruction_patterns[] =
{
   { "0000000000111100", &gen_ori_ccr },
   { "0000000001111100", &gen_ori_sr },
   { "00000000S2MMMXXX", &gen_ori, MODES_DATA | MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS },
   { "0000001000111100", &gen_andi_ccr },
   { "0000001001111100", &gen_andi_sr },
   { "00000010S2MMMXXX", &gen_andi, MODES_DATA | MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS },
   { "00000100S2MMMXXX", &gen_subi, MODES_DATA | MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS },
   { "00000110S2MMMXXX", &gen_addi, MODES_DATA | MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS },
   { "0000101000111100", &gen_eori_ccr },
   { "0000101001111100", &gen_eori_sr },
   { "00001010S2MMMXXX", &gen_eori, MODES_DATA | MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS },
   { "00001100S2MMMXXX", &gen_cmpi, MODES_ALL & ~(MODES_ADDR | MODES_IMM) },
   { "0000100000MMMXXX", &gen_btst_imm, MODES_DATA | MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS | MODES_PC },
   { "0000100001MMMXXX", &gen_bchg_imm, MODES_DATA | MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS },
   { "0000100010MMMXXX", &gen_bclr_imm, MODES_DATA | MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS },
   { "0000100011MMMXXX", &gen_bset_imm, MODES_DATA | MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS },
   { "0000???100MMMXXX", &gen_btst, MODES_DATA | MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS | MODES_PC },
   { "0000???101MMMXXX", &gen_bchg, MODES_DATA | MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS },
   { "0000???110MMMXXX", &gen_bclr, MODES_DATA | MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS },
   { "0000???111MMMXXX", &gen_bset, MODES_DATA | MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS },
   { "0000???1??001???", &gen_movep },
   { "0011???001MMMXXX", &gen_movea, MODES_ALL }, // Word (no byte-sized version)
   { "0010???001MMMXXX", &gen_movea, MODES_ALL }, // Long
   { "0001XXXMMMMMMXXX", &gen_move, MODES_DATA | MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS, MODES_ALL & ~MODES_ADDR}, // Byte: no address register as source
   { "00S3XXXMMMMMMXXX", &gen_move, MODES_DATA | MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS, MODES_ALL },
   { "0100000011MMMXXX", &gen_move_from_sr, MODES_DATA | MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS },
   { "0100010011MMMXXX", &gen_move_to_ccr,  MODES_ALL & ~MODES_ADDR },
   { "0100011011MMMXXX", &gen_move_to_sr, MODES_ALL & ~MODES_ADDR },
   { "01000000S2MMMXXX", &gen_negx, MODES_DATA | MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS },
   { "01000010S2MMMXXX", &gen_clr, MODES_DATA | MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS },
   { "01000100S2MMMXXX", &gen_neg, MODES_DATA | MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS },
   { "01000110S2MMMXXX", &gen_not, MODES_DATA | MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS },
   { "010010001?000???", &gen_ext },
   { "0100100000MMMXXX", &gen_nbcd, MODES_DATA | MODES_ADDR | MODES_ADDR_OFFSET | MODES_ABS },
   { "0100100001000???", &gen_swap },
   { "0100100001MMMXXX", &gen_pea, MODE_MASK(AddressRegisterIndirect) | MODES_ADDR_OFFSET | MODES_ABS | MODES_PC },
   { "0100101011111100", &gen_illegal },
   { "0100101011MMMXXX", &gen_tas, MODES_DATA | MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS },
   { "01001010S2MMMXXX", &gen_tst, MODES_ALL & ~(MODES_ADDR | MODES_IMM) },
   { "010011100100????", &gen_trap },
   { "0100111001010???", &gen_link },
   { "0100111001011???", &gen_unlk },
   { "010011100110????", &gen_move_usp },
   { "0100111001110000", &gen_reset },
   { "0100111001110001", &gen_nop },
   { "0100111001110010", &gen_stop },
   { "0100111001110011", &gen_rte },
   { "0100111001110101", &gen_rts },
   { "0100111001110110", &gen_rtr },
   { "0100111010MMMXXX", &gen_jsr, MODE_MASK(AddressRegisterIndirect) | MODES_ADDR_OFFSET | MODES_ABS | MODES_PC },
   { "0100111011MMMXXX", &gen_jmp, MODE_MASK(AddressRegisterIndirect) | MODES_ADDR_OFFSET | MODES_ABS | MODES_PC },
   { "010010001?MMMXXX", &gen_movem, MODE_MASK(AddressRegisterIndirect) | MODE_MASK(AddressRegisterIndirectPreDec) | MODES_ADDR_OFFSET | MODES_ABS },
   { "010011001?MMMXXX", &gen_movem, MODE_MASK(AddressRegisterIndirect) | MODE_MASK(AddressRegisterIndirectPostInc) | MODES_ADDR_OFFSET | MODES_ABS | MODES_PC },
   { "0100???111MMMXXX", &gen_lea,  MODE_MASK(AddressRegisterIndirect) | MODES_ADDR_OFFSET | MODES_ABS | MODES_PC },
   { "0100???110MMMXXX", &gen_chk, MODES_ALL & ~MODES_ADDR },
   { "0101???000MMMXXX", &gen_addq, MODES_DATA | MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS }, // Byte: no address register
   { "0101???001MMMXXX", &gen_addq, MODES_DATA | MODES_ADDR | MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS }, // Word
   { "0101???010MMMXXX", &gen_addq, MODES_DATA | MODES_ADDR | MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS }, // Long
   { "0101???100MMMXXX", &gen_subq, MODES_DATA | MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS }, // Byte: no address register
   { "0101???101MMMXXX", &gen_subq, MODES_DATA | MODES_ADDR | MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS }, // Word
   { "0101???110MMMXXX", &gen_subq, MODES_DATA | MODES_ADDR | MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS }, // Long
   { "0101????11MMMXXX", &gen_scc, MODES_DATA | MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS },
   { "0101????11001???", &gen_dbcc },
   { "01100000????????", &gen_bra },
   { "01100001????????", &gen_bsr },
   { "0110????????????", &gen_bcc }, 
   { "0111???0????????", &gen_moveq },
   { "1000???011MMMXXX", &gen_divu, MODES_ALL & ~MODES_ADDR },
   { "1000???111MMMXXX", &gen_divs, MODES_ALL & ~MODES_ADDR },
   { "1000???10000????", &gen_sbcd },
   { "1000???0S2MMMXXX", &gen_or, MODES_ALL & ~MODES_ADDR },
   { "1000???1S2MMMXXX", &gen_or, MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS },
   { "1001???000MMMXXX", &gen_sub, MODES_ALL & ~MODES_ADDR }, // Byte: no address register
   { "1001???001MMMXXX", &gen_sub, MODES_ALL }, // Word
   { "1001???010MMMXXX", &gen_sub, MODES_ALL }, // Long
   { "1001???1S2MMMXXX", &gen_sub, MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS },   
   { "1001???1S200????", &gen_subx },
   { "1001????11MMMXXX", &gen_suba, MODES_ALL },
   { "1011???1S2MMMXXX", &gen_eor, MODES_DATA | MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS },
   { "1011???1S2001???", &gen_cmpm },
   { "1011???0S2MMMXXX", &gen_cmp, MODES_ALL },
   { "1011????11MMMXXX", &gen_cmpa, MODES_ALL },
   { "1100???011MMMXXX", &gen_mulu, MODES_ALL & ~MODES_ADDR },
   { "1100???111MMMXXX", &gen_muls, MODES_ALL & ~MODES_ADDR },
   { "1100???10000????", &gen_abcd },
   { "1100???1??00????", &gen_exg },
   { "1100???0S2MMMXXX", &gen_and, MODES_ALL & ~MODES_ADDR },
   { "1100???1S2MMMXXX", &gen_and, MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS },
   { "1101???000MMMXXX", &gen_add, MODES_ALL & ~MODES_ADDR }, // Byte: no address register
   { "1101???001MMMXXX", &gen_add, MODES_ALL }, // Word
   { "1101???010MMMXXX", &gen_add, MODES_ALL }, // Long
   { "1101???1S2MMMXXX", &gen_add, MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS }, // TODO ADDA (w sign extension) for addr reg?
   { "1101???1S200????", &gen_addx },
   { "1101????11MMMXXX", &gen_adda, MODES_ALL },
   { "1110000?11MMMXXX", &gen_asd_mem, MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS },
   { "1110001?11MMMXXX", &gen_lsd_mem, MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS },
   { "1110010?11MMMXXX", &gen_roxd_mem, MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS },
   { "1110011?11MMMXXX", &gen_rod_mem, MODES_ADDR_IND | MODES_ADDR_OFFSET | MODES_ABS },
   { "1110????S2?00???", &gen_asd },
   { "1110????S2?01???", &gen_lsd },
   { "1110????S2?10???", &gen_roxd },
   { "1110????S2?11???", &gen_rod }
};

static bool pattern_match(Pattern* pattern, uint16_t opcode)
{
    assert(strlen(pattern->bits) == 16);

    uint16_t ea_modes = pattern->legal_ea_modes;
    uint16_t opcode_fragment;

    // Scan the bit pattern and check fragments against the opcode value
    int cursor = 0;
    while (cursor < 16)
    {
        switch (pattern->bits[cursor])
        {
        case '?':
            ++cursor;
            continue;

        case '0':
            if (BIT(opcode, 15 - cursor) != 0)
                return false;
            ++cursor;
            continue;

        case '1':
            if (BIT(opcode, 15 - cursor) != 1)
                return false;
            ++cursor;
            continue;

        case 'S':
            opcode_fragment = FRAGMENT(opcode, 15 - cursor, 14 - cursor);

            if (pattern->bits[cursor + 1] == '2')
            {
                if (opcode_fragment == 0b00 || opcode_fragment == 0b01 || opcode_fragment == 0b10)
                {
                    cursor += 2;
                    continue;
                }
                return false;
            }

            if (pattern->bits[cursor + 1] == '3')
            {
                if (opcode_fragment == 0b01 || opcode_fragment == 0b11 || opcode_fragment == 0b10)
                {
                    cursor += 2;
                    continue;
                }
                return false;
            }

            return false;

            // MMMXXX/XXXMMM
        case 'M':
        case 'X':

            opcode_fragment = FRAGMENT(opcode, 15 - cursor, 10 - cursor);

            // Swap the two halves if necessary
            opcode_fragment = pattern->bits[cursor] == 'X' ?
                (opcode_fragment & 0b000111) << 3 | (opcode_fragment & 0b111000) >> 3 :
                opcode_fragment;

            if ((opcode_fragment & 0b111000) == 0b000000 && (ea_modes & MODE_MASK(DataRegister)) ||
                (opcode_fragment & 0b111000) == 0b001000 && (ea_modes & MODE_MASK(AddressRegister)) ||
                (opcode_fragment & 0b111000) == 0b010000 && (ea_modes & MODE_MASK(AddressRegisterIndirect)) ||
                (opcode_fragment & 0b111000) == 0b011000 && (ea_modes & MODE_MASK(AddressRegisterIndirectPostInc)) ||
                (opcode_fragment & 0b111000) == 0b100000 && (ea_modes & MODE_MASK(AddressRegisterIndirectPreDec)) ||
                (opcode_fragment & 0b111000) == 0b101000 && (ea_modes & MODE_MASK(AddressRegisterIndirectDisplacement)) ||
                (opcode_fragment & 0b111000) == 0b110000 && (ea_modes & MODE_MASK(AddressRegisterIndirectIndexed)) ||
                opcode_fragment == 0b111010 && (ea_modes & MODE_MASK(ProgramCounterDisplacement)) ||
                opcode_fragment == 0b111011 && (ea_modes & MODE_MASK(ProgramCounterIndexed)) ||
                opcode_fragment == 0b111000 && (ea_modes & MODE_MASK(AbsoluteShort)) ||
                opcode_fragment == 0b111001 && (ea_modes & MODE_MASK(AbsoluteLong)) ||
                opcode_fragment == 0b111100 && (ea_modes & MODE_MASK(Immediate)))
            {
                cursor += 6;
                ea_modes = pattern->legal_ea_modes2;
                continue;
            }

            // TODO check legal modes*/

            return false;
        }

        return false;
    }

    return true;
}

Instruction* instruction_generate(M68k* context, uint16_t opcode)
{
    uint8_t pattern_count = sizeof(instruction_patterns) / sizeof(Pattern);

    for (uint8_t i = 0; i < pattern_count; ++i)
    {
        Pattern* pattern = &instruction_patterns[i];

        if (pattern_match(pattern, opcode))
        {
            Instruction* instr = pattern->generator(opcode);
            instr->opcode = opcode;
            return instr;
        }
    }

    return NULL;
}

Instruction* instruction_make(char* name, InstructionFunc func)
{
    Instruction* i = calloc(1, sizeof(Instruction));
    i->func = func;
    i->base_cycles = 0;

    // Dynamically allocate memory for the instruction's name
    // and copy the name argument whatever its source in order
    // to handle string literals and dynamic strings similarly.
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

uint8_t instruction_execute(Instruction* instr, M68k* ctx)
{
    // TODO compare w inline noop
    // Pre-execution actions
    if (instr->src != NULL && instr->src->pre_func != NULL)
        instr->src->pre_func(instr->src, ctx);
    if (instr->dst != NULL && instr->dst->pre_func != NULL)
        instr->dst->pre_func(instr->dst, ctx);

    uint8_t additional_cycles = instr->func(instr, ctx);

    // Post-execution actions
    if (instr->src != NULL && instr->src->post_func != NULL)
        instr->src->post_func(instr->src, ctx);
    if (instr->dst != NULL && instr->dst->post_func != NULL)
        instr->dst->post_func(instr->dst, ctx);

    return instr->base_cycles + additional_cycles;
}
