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

    // TODO not sure if those are fixed by the hardware or the ROMs just set them up
    m68k->status = 0x2704; // TODO not sure about Z
    m68k->address_registers[7] = 0xFFFE00;

    // Generate every possible opcode

    m68k->opcode_table = calloc(0x10000, sizeof(Instruction*));

    for (int opcode = 0; opcode < 0x10000; ++opcode)
    {
        // Manual breakpoint!
        if (opcode == 0x0200)
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

DecodedInstruction* m68k_decode(M68k* m, uint32_t instr_address)
{
    uint16_t opcode = m68k_read_w(m, instr_address);

    Instruction* instr = m->opcode_table[opcode];
    if (instr == NULL)
    {
        printf("Opcode %#06X cannot be found in the opcode table\n", opcode);
        return NULL;
    }

    DecodedInstruction* decoded = calloc(1, sizeof(DecodedInstruction));

    char* buffer = calloc(50, sizeof(char));
    int pos = sprintf(buffer, "%s", instr->name);

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
    pos += sprintf(buffer + pos, "%s ", size_symbol);

    if (instr->src != NULL)
        pos += operand_tostring(instr->src, instr_address, buffer + pos);

    if (instr->src != NULL && instr->dst != NULL)
        pos += sprintf(buffer + pos, ", ");

    if (instr->dst != NULL)
        pos += operand_tostring(instr->dst, instr_address, buffer + pos);

    buffer[pos] = '\0';

    decoded->mnemonics = buffer;
    return decoded;
}

uint32_t m68k_step(M68k* m)
{
    // Fetch the instruction
    uint16_t opcode = m68k_read_w(m, m->pc);
    Instruction* instr = m->opcode_table[opcode];

    // Manual breakpoint!
    if (m->pc == 0x338)
        printf("breakpoint\n");

    // 35C: weird move with unknown regmode

    if (instr == NULL)
    {
        printf("Opcode %#06X cannot be found in the opcode table\n", opcode);
    }
    else
    {
        //DecodedInstruction* d = m68k_decode(m, m->pc);
        //printf("%#06X %s\n", m->pc, d->mnemonics);

        if (m->instruction_callback != NULL)
            m->instruction_callback(m);

        m->cycles += instruction_execute(instr);

        m->pc += instr->total_length;
        // TODO can only address 2^24 bytes in practice

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