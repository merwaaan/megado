#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "bit_utils.h"
#include "instruction.h"
#include "m68k.h"
#include "operands.h"

M68k* m68k_make()
{
    M68k* m68k = calloc(1, sizeof(M68k));

    // Generate every possible opcode

    m68k->opcode_table = calloc(0x10000, sizeof(Instruction*));

    for (int opcode = 0; opcode < 0x10000; ++opcode)
    {
        // Manual breakpoint!
        if (opcode == 0x1031)
        {
            printf("breakpoint\n");
        }

        Instruction* instr = instruction_generate(m68k, opcode);

        if (!instruction_valid(instr))
        {
            instruction_free(instr);
            continue;
        }

        m68k->opcode_table[opcode] = instr;
    }

    return m68k;
}

void m68k_free(M68k* m)
{
    for (int opcode = 0; opcode < 0x10000; ++opcode)
        instruction_free(m->opcode_table[opcode]);

    free(m->opcode_table);
    free(m);
}

void m68k_initialize(M68k* m)
{
    m->status = 0x2700;
    m->address_registers[7] = m68k_read_l(m, 0); // TODO really required? Games seem to do this as part of their startup routine

    // Entry point
    m->pc = m68k_read_l(m, 4);

    m->prefetch_address = 0xFFFFFFFF; // Invalid value, will initiate the initial prefetch
}

DecodedInstruction* m68k_decode(M68k* m, uint32_t instr_address)
{
    // Save the current PC, will be restored at the end
    uint32_t initial_pc = m->pc;

    m->pc = instr_address;
    uint16_t opcode = m68k_fetch(m);

    Instruction* instr = m->opcode_table[opcode];
    if (instr == NULL)
    {
        // TODO restore pc
        printf("Opcode %#06X cannot be found in the opcode table\n", opcode);
        return NULL;
    }

    DecodedInstruction* decoded = calloc(1, sizeof(DecodedInstruction));

    int pos = sprintf(decoded->mnemonics, "%s", instr->name);

    char* size_symbol;
    switch (instr->size)
    {
    case Byte:
        size_symbol = ".b";
        break;
    case Word:
        size_symbol = ".w";
        break;
    case Long:
        size_symbol = ".l";
        break;
    default:
        size_symbol = " ";
    }
    pos += sprintf(decoded->mnemonics + pos, "%s ", size_symbol);

    if (instr->src != NULL)
        pos += operand_tostring(instr->src, decoded->mnemonics + pos);

    if (instr->src != NULL && instr->dst != NULL)
        pos += sprintf(decoded->mnemonics + pos, ", ");

    if (instr->dst != NULL)
        pos += operand_tostring(instr->dst, decoded->mnemonics + pos);

    decoded->mnemonics[pos] = '\0';

    // Restore the initial PC
    m->pc = initial_pc;

    return decoded;
}

uint32_t m68k_step(M68k* m)
{
    printf("%#06X\n", m->pc);
    // Manual breakpoint!
    if (m->pc == 0x2e1c) // 37a ok
        printf("breakpoint\n");

    // TODO Sonic@37E, D5 is wrong
    // TODO Sonic@29a8 weird status move

    // Fetch the instruction
    //uint32_t instr_pc = m->pc;
    m->instruction_register = m68k_fetch(m);
    Instruction* instr = m->opcode_table[m->instruction_register];

    // 35C: weird move with unknown regmode

    if (instr == NULL)
    {
        printf("Opcode %#06X cannot be found in the opcode table\n", m->instruction_register);
    }
    else
    {
        /*DecodedInstruction* d = m68k_decode(m, m->pc - 2);
        if (d != NULL)
            printf("%s\n", d->mnemonics);*/

        //if (m->instruction_callback != NULL)
        //    m->instruction_callback(m);

        m->cycles += instruction_execute(instr);

        //free(d);
    }

    return m->pc;
}

uint32_t m68k_read(M68k* m, Size size, uint32_t address)
{
    switch (size)
    {
    case Byte:
        return m68k_read_b(m, address);
    case Word:
        return m68k_read_w(m, address);
    case Long:
        return m68k_read_l(m, address);
    default:
        return 0xFF; // TODO error?
    }
}

void m68k_write(M68k* m, Size size, uint32_t address, uint32_t value)
{
    switch (size)
    {
    case Byte:
        m68k_write_b(m, address, value);
        break;
    case Word:
        m68k_write_w(m, address, value);
        break;
    case Long:
        m68k_write_l(m, address, value);
        break;
    }
}

uint16_t m68k_fetch(M68k* m)
{
    // If the PC jumped, discard the prefetch queue
    if (m->pc != m->prefetch_address)
    {
        uint16_t word = m68k_read_w(m, m->pc);
        m->pc = m->prefetch_address = m->pc + 2;

        m->prefetch_queue[0] = m68k_read_w(m, m->pc);
        m->prefetch_queue[1] = m68k_read_w(m, m->pc + 2);

        return word;
    }

    // Otherwise, shift the prefetch queue
    uint16_t word = m->prefetch_queue[0];
    m->prefetch_queue[0] = m->prefetch_queue[1];
    m->pc = m->prefetch_address = m->pc + 2;
    m->prefetch_queue[1] = m68k_read_w(m, m->pc + 2);
    return word;
}
