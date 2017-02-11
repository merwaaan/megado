#pragma once

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

#define CARRY_SET(context, b) context->status = BIT_CHG(context->status, CARRY_BIT, b) // TODO optim: do not use chg
#define OVERFLOW_SET(context, b) context->status = BIT_CHG(context->status, OVERFLOW_BIT, b)
#define ZERO_SET(context, b) context->status = BIT_CHG(context->status, ZERO_BIT, b)
#define NEGATIVE_SET(context, b) context->status = BIT_CHG(context->status, NEGATIVE_BIT, b)
#define EXTENDED_SET(context, b) context->status = BIT_CHG(context->status, EXTENDED_BIT, b)

#define SIGN_EXTEND_B(x) (((x) & 0xFF) | (BIT((x), 7) ? 0xFF00 : 0))
#define SIGN_EXTEND_W(x) (((x) & 0xFFFF) | (BIT((x), 15) ? 0xFFFF0000 : 0))
#define SIGN_EXTEND_B_L(x) (((x) & 0xFF) | (BIT((x), 7) ? 0xFFFFFFFFFFFFFF00 : 0))

struct Instruction;
struct DecodedInstruction;
struct M68k;

typedef void(*CallbackFunc)(struct M68k*);

typedef struct M68k {
    int32_t data_registers[8];
    uint32_t address_registers[8];
    uint16_t status;
    uint32_t pc;
    uint64_t cycles;

    // Prefetching pipeline containing the two next words of the program stream
    uint16_t prefetch_queue[2];
    uint32_t prefetch_address;
    uint16_t instruction_register; // Instruction currently being decoded

    struct Instruction** opcode_table;

    // Callbacks
    CallbackFunc instruction_callback;

    // Arbitrary user-defined data associated with this M68000 instance
    void* user_data;
} M68k;

typedef struct Instruction* (GenFunc)(uint16_t opcode, M68k* context);

typedef struct {
    uint16_t pattern;
    uint16_t mask;
    GenFunc* generator;
} Pattern;

M68k* m68k_make();
void m68k_free(M68k*);

struct DecodedInstruction* m68k_decode(M68k*, uint32_t pc);

// Prepare the CPU for execution (stack pointer, program start, initial prefetch...)
void m68k_initialize(M68k*);

// Execute one instruction and return the current program counter value
uint32_t m68k_step(M68k*); 

uint32_t m68k_read(M68k*, Size size, uint32_t address);
void m68k_write(M68k*, Size size, uint32_t address, uint32_t value);

// Return the word currently under the program counter
// and make it advance.
//
// All reads should go through this function to properly
// emulate the CPU's two-words long prefetching pipeline.
//
// http://pasti.fxatari.com/68kdocs/68kPrefetch.html
// http://ataristeven.exxoshost.co.uk/txt/Prefetch.txt
// "Assembly Language and Systems Programming for the M68000 Family", p. 790
uint16_t m68k_fetch(M68k* m);

// -----
// The following I/O functions must be implemented
// -----

uint8_t m68k_read_b(M68k*, uint32_t address);
uint16_t m68k_read_w(M68k*, uint32_t address);
uint32_t m68k_read_l(M68k*, uint32_t address);

void m68k_write_b(M68k*, uint32_t address, uint8_t value);
void m68k_write_w(M68k*, uint32_t address, uint16_t value);
void m68k_write_l(M68k*, uint32_t address, uint32_t value);
