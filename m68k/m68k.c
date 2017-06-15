#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "bit_utils.h"
#include "instruction.h"
#include "m68k.h"
#include "operands.h"

#ifdef DEBUG
#define LOG_M68K(...) printf(__VA_ARGS__)
#else
#define LOG_M68K(...)
#endif

M68k* m68k_make()
{
    M68k* m68k = calloc(1, sizeof(M68k));
    m68k->pending_interrupt = -1;

    m68k->opcode_table = calloc(0x10000, sizeof(Instruction*));

    // Generate every possible opcode
    for (int opcode = 0; opcode < 0x10000; ++opcode)
        m68k->opcode_table[opcode] = instruction_generate(m68k, opcode);
        
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
        m->pc = initial_pc;
        LOG_M68K("Opcode %#06X cannot be found in the opcode table\n", opcode);

        return NULL;
    }

    DecodedInstruction* decoded = calloc(1, sizeof(DecodedInstruction));
    decoded->mnemonics = calloc(100, sizeof(char));
    decoded->length = 2;

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
    {
        pos += operand_tostring(instr->src, decoded->mnemonics + pos);
        decoded->length += operand_length(instr->src);
    }

    if (instr->src != NULL && instr->dst != NULL)
        pos += sprintf(decoded->mnemonics + pos, ", ");

    if (instr->dst != NULL)
    {
        pos += operand_tostring(instr->dst, decoded->mnemonics + pos);
        decoded->length += operand_length(instr->dst);
    }

    decoded->mnemonics[pos] = '\0';

    // Restore the initial PC
    m->pc = initial_pc;

    return decoded;
}

uint32_t m68k_run_cycles(M68k* m, int cycles)
{
    /*while (cycles > 0)
        cycles -= m68k_step(m);*/

        // TODO temporary version for tracking non-timed instructions
    while (cycles > 0)
    {
        int c = m68k_step(m);

        if (c == 0)
        {
            LOG_M68K("WARNING, instruction took ZERO CYCLES\n");
            c = 10; // we don't want to block the execution
        }

        cycles -= c;
    }

    // TODO in practice, should only consume available cycles and return the remainder
    return cycles;
}

bool breakpoint_triggered = true;

uint8_t m68k_step(M68k* m)
{
    if (m->pc == m->breakpoint)
        breakpoint_triggered = true;

    // Fetch the instruction
    m->instruction_address = m->pc;
    m->instruction_register = m68k_fetch(m);
    Instruction* instr = m->opcode_table[m->instruction_register];

    // 35C: weird move with unknown regmode

    if (instr == NULL)
    {
        LOG_M68K("\tOpcode %#06X cannot be found in the opcode table\n", m->instruction_register);

        return 0;
    }

#ifdef DEBUG
    DecodedInstruction* d = breakpoint_triggered || true ? m68k_decode(m, m->instruction_address) : NULL;
    if (d != NULL)
        printf("%#06X   %s\n", m->pc - 2, d->mnemonics);
    decoded_instruction_free(d);
#endif

    //if (m->instruction_callback != NULL)
    //    m->instruction_callback(m);

    int cycles = instruction_execute(instr);
    m->cycles += cycles;

    m68k_handle_interrupt(m);

    return cycles;
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

#define IRQ_VECTOR_OFFSET 96

// TODO filter/record interrupt but only service it on the next step

void m68k_request_interrupt(M68k* m, uint8_t level)
{
    LOG_M68K("Interrupt %d requested\n", level);

    // Ignore low-priority interrupts (except for level 7, which is non-maskable)
    if (level <= STATUS_INTERRUPT_MASK(m) && level != 7)
    {
        LOG_M68K("\tInterrupt ignored, current interrupt mask: %d\n", STATUS_INTERRUPT_MASK(m));

        return;
    }

    m->pending_interrupt = level;
}

void m68k_handle_interrupt(M68k* m)
{
    if (m->pending_interrupt < 0)
        return;

    LOG_M68K("Interrupt %d handled\n", m->pending_interrupt);

    // Push the current PC onto the stack
    m->address_registers[7] -= 4;
    m68k_write_l(m, m->address_registers[7], m->pc);

    // Push the status register onto the stack
    m->address_registers[7] -= 2;
    m68k_write_w(m, m->address_registers[7], m->status);

    // Update the interrupt mask
    m->status = (m->status & 0xF8FF) | m->pending_interrupt << 8;

    m->pc = m68k_read_l(m, IRQ_VECTOR_OFFSET + m->pending_interrupt * 4);

    m->pending_interrupt = -1;
}

void reti(Instruction* i)
{
    i->context->status = m68k_read_l(i->context, i->context->address_registers[7]);
    i->context->address_registers[7] += 4;

    i->context->pc = m68k_read_l(i->context, i->context->address_registers[7]);
    i->context->address_registers[7] += 4;
}
