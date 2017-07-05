#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "bit_utils.h"
#include "instruction.h"
#include "m68k.h"
#include "operands.h"
#include "../genesis.h"

#ifdef DEBUG
#define LOG_M68K(...) printf(__VA_ARGS__)
#else
#define LOG_M68K(...)
#endif

Instruction** opcode_table;

M68k* m68k_make(Genesis* g)
{
    M68k* m68k = calloc(1, sizeof(M68k));
    m68k->genesis = g;
    return m68k;
}

void m68k_free(M68k* m)
{
    free(m);
}

void m68k_initialize(M68k* m)
{
    m->status = 0x2700;
    m->address_registers[7] = m68k_read_l(m, 0); // Stack pointer
    m->pc = m68k_read_l(m, 4); // Program start

    m->cycles = 0;
    m->pending_interrupt = -1;
    m->prefetch_address = 0xFFFFFFFF; // Invalid value, will initiate the initial prefetch

    // Reset breakpoints
    for (uint8_t i = 0; i < BREAKPOINTS_COUNT; ++i)
        m->breakpoints[i] = (Breakpoint) { false, 0 };
}

DecodedInstruction* m68k_decode(M68k* m, uint32_t instr_address)
{
    // Save state that can be modified by instructions; will be restored on exit
    uint32_t _pc = m->pc;
    uint16_t _instruction_register = m->instruction_register;
    uint32_t _instruction_address = m->instruction_address;

    m->instruction_address = instr_address;
    m->pc = instr_address;
    uint16_t opcode = m68k_fetch(m);
    m->instruction_register = opcode;

    Instruction* instr = opcode_table[opcode];
    DecodedInstruction* decoded = NULL;
    if (instr == NULL)
    {
        LOG_M68K("Opcode %#06X cannot be found in the opcode table\n", opcode);
        goto bail;
    }

    decoded = calloc(1, sizeof(DecodedInstruction));
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
        pos += operand_tostring(instr->src, m, decoded->mnemonics + pos);
        decoded->length += operand_length(instr->src);
    }

    if (instr->src != NULL && instr->dst != NULL)
        pos += sprintf(decoded->mnemonics + pos, ", ");

    if (instr->dst != NULL)
    {
        pos += operand_tostring(instr->dst, m, decoded->mnemonics + pos);
        decoded->length += operand_length(instr->dst);
    }

    decoded->mnemonics[pos] = '\0';

bail:
    // Restore overriden context state
    m->pc = _pc;
    m->instruction_register = _instruction_register;
    m->instruction_address = _instruction_address;

    return decoded;
}

void decoded_instruction_free(DecodedInstruction* decoded)
{
    if (decoded == NULL)
        return;

    free(decoded->mnemonics);
    free(decoded);
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
            //LOG_M68K("WARNING, instruction took ZERO CYCLES\n");
            c = 10; // we don't want to block the execution
        }

        cycles -= c;

        // Exit early if the emulation has been paused
        if (m->genesis->status != Status_Running)
            break;
    }

    // TODO in practice, should only consume available cycles and return the remainder
    return cycles;
}

uint8_t m68k_step(M68k* m)
{
    // Pause on breakpoints
    // TODO only in DEBUG builds? check perf
    Breakpoint* breakpoint = m68k_get_breakpoint(m, m->pc);
    if (breakpoint != NULL)
    {
        // If the breakpoint has already been touched, do not pause again
        if (breakpoint == m->active_breakpoint)
        {
            m->active_breakpoint = NULL;
        }
        else
        {
            m->active_breakpoint = breakpoint;
            m->genesis->status = Status_Pause;
            return 0;
        }
    }

    // Fetch the instruction
    m->instruction_address = m->pc;
    m->instruction_register = m68k_fetch(m);
    Instruction* instr = opcode_table[m->instruction_register];

    // 35C: weird move with unknown regmode

    if (instr == NULL)
    {
        LOG_M68K("\tOpcode %#06X cannot be found in the opcode table\n", m->instruction_register);
        return 0;
    }

#ifdef DEBUG
    DecodedInstruction* d = m68k_decode(m, m->instruction_address);

    if (d != NULL)
        printf("%#06X [%0X] %s\n",// %0X %0X %0X %0X %0X %0X %0X | A %0X %0X %0X %0X %0X %0X %0X %0X %0X\n",
            m->pc - 2, m->instruction_register, d->mnemonics);
            //m->data_registers[0], m->data_registers[1], m->data_registers[2], m->data_registers[3], m->data_registers[4], m->data_registers[5], m->data_registers[6], m->data_registers[7],
            //m->address_registers[0], m->address_registers[1], m->address_registers[2], m->address_registers[3], m->address_registers[4], m->address_registers[5], m->address_registers[6], m->address_registers[7]);

    decoded_instruction_free(d);
#endif

    m->instruction_count += 1;

    //if (m->instruction_callback != NULL)
    //    m->instruction_callback(m);

    int cycles = instruction_execute(instr, m);
    m->cycles += cycles;

    m68k_handle_interrupt(m);

    return cycles;
}

inline uint16_t m68k_fetch(M68k* m)
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

void m68k_toggle_breakpoint(M68k* m, uint32_t address)
{
    // Toggle existing breakpoints
    for (int i = 0; i < BREAKPOINTS_COUNT; ++i)
        if (m->breakpoints[i].address == address)
        {
            m->breakpoints[i].enabled = !m->breakpoints[i].enabled;
            return;
        }

    // Otherwise, use the first free slot
    for (int i = 0; i < BREAKPOINTS_COUNT; ++i)
        if (!m->breakpoints[i].enabled)
        {
            m->breakpoints[i].enabled = true;
            m->breakpoints[i].address = address;
            return;
        }

    printf("Warning, no free slots for a new breakpoint at %0X", address); // TODO would be nice to output warnings/errors via the UI too
}

Breakpoint* m68k_get_breakpoint(M68k* m, uint32_t address)
{
    for (int i = 0; i < BREAKPOINTS_COUNT; ++i)
        if (m->breakpoints[i].enabled && m->breakpoints[i].address == address)
            return &m->breakpoints[i];

    return NULL;
}
