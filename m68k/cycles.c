#include "cycles.h"
#include "instruction.h"
#include "operands.h"

uint8_t cycles_ea_calculation_table[2][12] =
{
    { 0, 0, 4, 4,  6,  8, 10,  8, 12,  8, 10, 4 }, // Byte, Word
    { 0, 0, 8, 8, 10, 12, 14, 12, 16, 12, 14, 8 }  // Long
};

uint8_t cycles_move_table[2][12][9] =
{
    // Byte, Word
    {
        {  4,  4,  8,  8,  8, 12, 14, 12, 16 },
        {  4,  4,  8,  8,  8, 12, 14, 12, 16 },
        {  8,  8, 12, 12, 12, 16, 18, 16, 20 },
        {  8,  8, 12, 12, 12, 16, 18, 16, 20 },
        { 10, 10, 14, 14, 14, 18, 20, 18, 22 },
        { 12, 12, 16, 16, 16, 20, 22, 20, 24 },
        { 14, 14, 18, 18, 18, 22, 24, 22, 26 },
        { 12, 12, 16, 16, 16, 20, 22, 20, 24 },
        { 16, 16, 20, 20, 20, 24, 26, 24, 28 },
        { 12, 12, 16, 16, 16, 20, 22, 20, 24 },
        { 14, 14, 18, 18, 18, 22, 24, 22, 26 },
        {  8,  8, 12, 12, 12, 16, 18, 16, 20 }
    },

    // Long
    {
        {  4,  4, 12, 12, 12, 16, 18, 16, 20 },
        {  4,  4, 12, 12, 12, 16, 18, 16, 20 },
        { 12, 12, 20, 20, 20, 24, 26, 24, 28 },
        { 12, 12, 20, 20, 20, 24, 26, 24, 28 },
        { 14, 14, 22, 22, 22, 26, 28, 26, 30 },
        { 16, 16, 24, 24, 24, 28, 30, 28, 32 },
        { 18, 18, 26, 26, 26, 30, 32, 30, 34 },
        { 16, 16, 24, 24, 24, 28, 30, 28, 32 },
        { 20, 20, 28, 28, 28, 32, 34, 32, 36 },
        { 16, 16, 24, 24, 24, 28, 30 ,28, 32 },
        { 18, 18, 26, 26, 26, 30, 32, 30, 34 },
        { 12, 12, 20, 20, 20, 24, 26, 24, 28 }
    }
};

// TODO  ** The base time of six clock periods is increased to eight		
//if the effective address mode is register direct or
//immediate(effective address time should also be added)
// http://oldwww.nvg.ntnu.no/amiga/MC680x0_Sections/timstandard.HTML
uint8_t cycles_standard_instruction(Instruction* i, uint8_t ea_an_cycles, uint8_t ea_dn_cycles, uint8_t dn_ea_cycles)
{
    uint8_t cycles = 0;

    if (i->src->type == DataRegister)
        cycles = dn_ea_cycles;
    else if (i->dst->type == AddressRegister)
        cycles = ea_an_cycles;
    else if (i->dst->type == DataRegister)
        cycles = ea_dn_cycles;

    cycles += i->src != NULL ? cycles_ea_calculation_table[i->size][i->src->type] : 0;
    cycles += i->dst != NULL ? cycles_ea_calculation_table[i->size][i->dst->type] : 0;

    return cycles;
}

uint8_t cycles_single_operand_instruction(Instruction* i, uint8_t register_cycles, uint8_t memory_cycles)
{
    if (i->dst->type == DataRegister || i->dst->type == AddressRegister)
        return register_cycles;
    else
        return memory_cycles + cycles_ea_calculation_table[i->size][i->dst->type];
}

uint8_t cycles_bit_manipulation_instruction(struct Instruction* i, uint8_t register_cycles, uint8_t memory_cycles)
{
    if (i->src->type == AddressRegister || i->src->type == DataRegister)
        return register_cycles;

    return memory_cycles;
}
