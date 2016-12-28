#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "bit_utils.h"
#include "instruction.h"
#include "m68k.h"
#include "operands.h"

M68k* m68k_make(uint8_t* memory)
{
    M68k* m68k = calloc(1, sizeof(M68k));
    m68k->memory = memory;

    // TODO temp
    for (int i = 0; i < 8; ++i)
        m68k->data_registers[i] = i;
    for (int i = 0; i < 8; ++i)
        m68k->address_registers[i] = 100 + i;

    m68k->opcode_table = calloc(0x10000, sizeof(Instruction*));

    // Generate every possible opcode
    for (int opcode = 0; opcode < 0x10000; ++opcode)
    {
        Instruction* instr = instruction_generate(opcode, m68k);

        if (!instruction_valid(instr))
        {
            instruction_free(instr);
            continue;
        }

        m68k->opcode_table[opcode] = instr;

        /*printf("opcode %#04X: %s", opcode, instr->name);
        for (int i = 0; i < instr->operand_count; ++i)
            printf(" %s", operand_tostring(instr->operands[i]));
        printf("\n");*/
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

DecodedInstruction* m68k_decode(M68k* m, uint32_t pc)
{
    printf("...%p %d...\n", m, m->data_registers[1]);
    uint16_t opcode = (m->memory[m->pc] << 8) | m->memory[m->pc + 1];
    printf("Decoding opcode %#06X [%#010X]...\n", opcode, pc);

    Instruction* instr = m->opcode_table[opcode];
    if (instr == NULL)
    {
        printf("Opcode %#06X cannot be found in the opcode table\n", opcode);
        return NULL;
    }

    DecodedInstruction* decoded = calloc(1, sizeof(DecodedInstruction));

    char* buffer = calloc(50, sizeof(char));
    int pos = sprintf(buffer, "%s ", instr->name);

    for (int i = 0; i < instr->operand_count; ++i)
        pos += operand_tostring(instr->operands[i], buffer + pos);

    buffer[pos] = '\0';

    printf("Decoded \"%s\"\n", buffer);
    decoded->mnemonics = buffer;
    return decoded;
}

uint32_t m68k_step(M68k* m)
{
    uint16_t opcode = (m->memory[m->pc] << 8) | m->memory[m->pc + 1];
    return m68k_execute(m, opcode);
}

uint32_t m68k_execute(M68k* m, uint16_t opcode)
{
    Instruction* instr = m->opcode_table[opcode];
    if (instr == NULL)
        printf("Opcode %#08X cannot be found in the opcode table\n", opcode);
    else
        instr->func(instr);

    return m->pc;
}

void m68k_push(int value)
{

}

int m68k_pop()
{

}

void m68k_jump(int address)
{

}
