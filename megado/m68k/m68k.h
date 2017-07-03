#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "bit_utils.h"

#define CARRY_BIT 0
#define OVERFLOW_BIT 1
#define ZERO_BIT 2
#define NEGATIVE_BIT 3
#define EXTENDED_BIT 4

#define CARRY(context) BIT(context->status, CARRY_BIT)
#define OVERFLOW(context) BIT(context->status, OVERFLOW_BIT)
#define ZERO(context) BIT(context->status, ZERO_BIT)
#define NEGATIVE(context) BIT(context->status, NEGATIVE_BIT)
#define EXTENDED(context) BIT(context->status, EXTENDED_BIT)

#define CARRY_SET(context, b) context->status = BIT_CHG(context->status, CARRY_BIT, b) // TODO optim: do not use chg (or optim bchg)
#define OVERFLOW_SET(context, b) context->status = BIT_CHG(context->status, OVERFLOW_BIT, b)
#define ZERO_SET(context, b) context->status = BIT_CHG(context->status, ZERO_BIT, b)
#define NEGATIVE_SET(context, b) context->status = BIT_CHG(context->status, NEGATIVE_BIT, b)
#define EXTENDED_SET(context, b) context->status = BIT_CHG(context->status, EXTENDED_BIT, b)

#define STATUS_TRACE_MODE(context) FRAGMENT((context)->status, 15, 14)
#define STATUS_SUPERVISOR_MODE(context) BIT((context)->status, 13)
#define STATUS_INTERRUPT_MASK(context) FRAGMENT((context)->status, 10, 8)

#define BREAKPOINTS_COUNT 3

struct Instruction;
struct DecodedInstruction;
struct Genesis;
struct M68k;

typedef struct Breakpoint
{
    bool enabled;
    uint32_t address;
    // TODO hit counter could be useful
} Breakpoint;

typedef struct M68k
{
    struct Genesis* genesis;

    uint32_t pc;
    uint16_t status;
    int32_t data_registers[8];
    uint32_t address_registers[8];

    // The CPU uses different stack pointers in supervisor/user mode.
    // Those variable only hold the value of the disabled mode while
    // the CPU is in the other one. 
    // The active stack pointer value must always be accessed via A7.
    uint32_t ssp;
    uint32_t usp; // TODO really necessary?

    uint64_t cycles;

    // Level of any pending interrupt (negative values means no interrupts)
    int pending_interrupt;

    // Prefetching pipeline
    uint16_t prefetch_queue[2]; // The two next words of the program stream
    uint32_t prefetch_address; // Address of the value at the head of the queue
    uint16_t instruction_register; // Instruction currently being decoded
    uint32_t instruction_address; // Instruction currently being decoded

    // Breakpoints
    Breakpoint breakpoints[BREAKPOINTS_COUNT]; // The emulation will pause when the PC reaches one of those addresses
    Breakpoint* active_breakpoint; // The breakpoint currently blocking the emulation
} M68k;

typedef struct Instruction* (GenFunc)(uint16_t opcode);

// TODO move
typedef struct Pattern {

    // Unique bit pattern of the instruction.
    //
    // eg. ANDI's opcode is 0000 0010 SSMM MXXX
    //     ANDI's pattern is $0200 (0000 0010 0000 0000)
    uint16_t pattern;

    // Bits to check for the pattern (the fixed part of the opcode).
    //
    // eg. ANDI's mask is $FF00 (1111 1111 0000 0000)
    uint16_t mask;

    // Function that returns an instance of the instruction
    // setup with appropriate operands depending on the opcode.
    GenFunc* generator;

    // Masks describing legal addressing modes for both operands.
    uint16_t src_addressing_modes;
    uint16_t dst_addressing_modes;
} Pattern;

M68k* m68k_make(struct Genesis*);
void m68k_free(M68k*);

// Prepare the CPU for execution (setup stack pointer, program start, initial prefetch...)
void m68k_initialize(M68k*);

uint8_t m68k_step(M68k*); // Execute one instruction, return cycles taken
uint32_t m68k_run_cycles(M68k*, int); // Execute n cycles worth of instructions, return cycles that were not consumed

// Return the word currently under the program counter
// and make it advance.
//
// All reads should go through this function to properly
// emulate the CPU's two-words long prefetching pipeline.
//
// http://pasti.fxatari.com/68kdocs/68kPrefetch.html
// http://ataristeven.exxoshost.co.uk/txt/Prefetch.txt
// "Assembly Language and Systems Programming for the M68000 Family", p. 790
inline uint16_t m68k_fetch(M68k* m);

// Interrupt handling
//
// Requested interrupts will be filtered depending on the current mask.
// Pending interrupts will be serviced after the current instruction.
void m68k_request_interrupt(M68k* m, uint8_t level);
void m68k_handle_interrupt(M68k* m);

struct DecodedInstruction* m68k_decode(M68k*, uint32_t pc);

// Breakpoint handling
void m68k_toggle_breakpoint(M68k*, uint32_t address); // Add/Remove a breakpoint at the given address
Breakpoint* m68k_get_breakpoint(M68k*, uint32_t address); // Return a possible enabled breakpoint at the given address

// -----
// I/O functions
//
// Note: While the 68000 handles 32-bit addresses, its address
// bus is 24-bit. The I/O functions must take that into account
// (eg. by masking the addresses by 0xFFFFFF).
// -----

uint32_t m68k_read(M68k*, Size size, uint32_t address);
uint8_t m68k_read_b(M68k*, uint32_t address);
uint16_t m68k_read_w(M68k*, uint32_t address);
uint32_t m68k_read_l(M68k*, uint32_t address);

void m68k_write(M68k*, Size size, uint32_t address, uint32_t value);
void m68k_write_b(M68k*, uint32_t address, uint8_t value);
void m68k_write_w(M68k*, uint32_t address, uint16_t value);
void m68k_write_l(M68k*, uint32_t address, uint32_t value);

static struct Instruction** opcode_table;
