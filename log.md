# Logs

## 04/02/2016

### Rethinking the CPU pipeline - **DRAFT**

#### Issue

Since the last log, I have implemented the most common opcodes and the emulator can advance through a good portion of some Genesis roms. However, I'm realizing that my current approach for decoding instructions and advancing through programs might be inappropriate.

Currently, each possible instruction variant is precomputed and packed in a structure that holds metadata (name, length, timings) and operands.

```
struct Instruction {
    char* name;

    // Implementation
    InstructionFunc* func;

    // Operands
    struct Operand* src;
    struct Operand* dst;

    // Size of the operation (byte, word, long)
    Size size;

    // Instruction length in bytes
    uint8_t length;
}
```

An operand basically is some additional metadata (addressing mode) as well as a pointer to a function to get the target data, and another to a function for modifying said data.

```
struct Operand
{
    AddressingMode mode;

    // Functions to access/modify the operand's data
    GetFunc get;
    SetFunc set;
    
    // Additional metadata ...
}
```

For instance, the *Address register indirect* mode functions looks like:

```
uint8_t address_register_indirect_get(Operand* op, uint32_t address)
{
  uint32_t address = M68K->address_registers[op->reg_number];
  return MEMORY[address];
}

void address_register_indirect_set(Operand* op, uint32_t address, uint8_t value)
{
  // ...
  MEMORY[address] = value;
}
```

This approach seems like it would reduce addressing modes to a small set of get/set functions; clean and easy to manage. However, things gets a little messy for operands that target data located in memory. For example, the instruction `NOT ($ABCD).w` (Invert the bits of the value at address `ABCD`) is encoded as `4639 ABCD`: `4639` is the NOT opcode and `ABCD` is the operand data as an extension word. The *Absolute short* mode used in this instruction is currently coded like:

```
uint16_t absolute_short_get(Operand* op, uint32_t address)
{
  // The address is stored just after the 2-bits long opcode
  uint16_t address = MEMORY[INSTRUCTION_ADDRESS + 2] << 8 | MEMORY[INSTRUCTION_ADDRESS + 3];
  return MEMORY[address];
}
```

The issue here is that the operand data is stored as an extension word that follows the opcode, so the code that extracts the data is dependent on the global context of execution and not just on the operand's internal metadata. To illustrate this issue, let's look at `MOVE ($ABCD) ($1234)` (move the data at address `ABCD` to address `1234`). It is is encoded as `11F8 1234 ABCD`: `11F8` is MOVE, `1234` is the destination address, `ABCD` is the source address. Both addresses are stored as extension words. The `absolute_short_get` is thus incorrect because the value targeted by both operands will be `1234`, because of the fixed +2 offset.

A possible solution to this issue would be to track which operands have been extracted, with some kind of virtual cursor but this would complexify the operand functions and couple the function to the execution context furthermore. I feel like the current design is just wrong.

### Rework

The way that the CPU reads the program stream is not consistent with what a real 68000 would do. In practice, the CPU cannot move back and forth around its program counter (PC) to pick data anywhere. The PC is the eye of the CPU and the 68000 can only decodes what's under its PC. This is the same for the operands data, the CPU need to advance its PC past the opcode word to read extension words.

The emulator should reflect this mechanics. First, this would fix the aforementioned issues. Second, this is a step towards a more accurate emulation since some programs rely on prefetching to work... TODO

References
- http://pasti.fxatari.com/68kdocs/68kPrefetch.html
- http://ataristeven.exxoshost.co.uk/txt/Prefetch.txt
- "Assembly Language and Systems Programming for the M68000 Family", p. 790

## 11/12/2016

I now want to want the plug the M68000 emulator to the JS UI so I looked more closely at [emscripten](http://kripken.github.io/emscripten-site/) the past few days. I experienced *a lot* of trouble compiling my M68000 C library to JS and I'm not there yet.

The emscripten website states that:

 > [...] Building large projects with Emscripten is very easy. Emscripten provides two simple scripts that configure your makefiles to use emcc as a drop-in replacement for gcc.

Those two wrapper scripts invoke the `./configure` and `make` commands after tweaking the build environment to use emscripten's compiler (eg. `emcc` instead of `gcc`).

```
emconfigure ./configure
emmake make
```
In my environment, using those tools have no effect whatsoever on the make output. The produced files should be LLVM bytecode but a binary diff confirmed that I end up with the same data with or without the wrappers. I also noticed that various emscripten test projects using this toolchain have platform-specific issues at different points of their builds. So, All in all, I'm not very confident in those two scripts.

Using `emcc` manually produces the expected bytecode, though, so I'll just write my own makefile.

## 11/11/2016

My process when implementing a new instruction is as follows:

- Add the opcode mask to the generation table (opcode pattern, generation function)
- Implement the generation function ("those bits are operand 1", "this bit is the size", etc...)
- Implement the actual instruction
- Write unit tests for the instruction

I try to do this once a day, when I have a break, but it's starting to get boring. I originally planned to emulate the whole instruction of the M68K before moving on to other area aspects of the emulator such as graphics. I might rethink this strategy. It would just be more fun to have enough code to be able to load a ROM and step into te code of a game. I'll implement instructions when I need them.

Anyway, right now I cover approximately 25% of the opcodes so the project is advancing at a good pace.

On another front, I started to build a tiny UI to debug the emulator. I used [React](https://facebook.github.io/react/) to write a memory viewer and a disassembler. Nothing is plugged to the emulator yet but the components are ready. As usual, building custom UI bits with React is a breeze, I really like this framework.

The nice surprise comes from [webpack](https://webpack.github.io/docs/), which I used to bundle all the JSX, HTML and CSS. This is so much nicer than Gulp/Grunt where you have a dozen different recipes for the same task and where your build toolchain globally feels like a patch-up job.

## 11/28/2016

I wrote a basic proof of concept for generating some instructions. The gist of it:

- A table holds the patterns distinguishing each instruction, each pattern is associated with a *generator function* (eg. `gen_add()`, `gen_or()`, ...)
- We loop over all possible opcode values in `[0, 0xFFFF]` and call the associated generator for each opcode
- Generators are short pieces of code that extract the relevant fragments from the bit pattern (operands, size, condition...)
- The return value of a generator is a structure that holds a pointer to the function emulating the instruction (eg. `add()`, `or()`, ...) as well as pointers to functions emulating the addressing mode of this variation of the instruction

## 11/26/2016

I'll start by coding the Motorola 68000 CPU (m68k). The first step is to look at some documentations. I found a [nice table](http://goldencrystal.free.fr/M68kOpcodes-v2.3.pdf) with all its opcodes and this seems more manageable than the GameBoy's Z80.

I need to think about a nice way to emulate all those instructions. Writing all the opcode variations by hand would take weeks and be no fun at all.

In my previous GameBoy emulator, I abstracted addressing modes and instructions to factorize all the existing variations (eg. one add() function compatible with a set of addressing mode proxies). It was ok but the virtualization was bad for performance and I may not get away with this approach on this particular project (gotta go fast!).

In my unfinished Master System emulator, I generated C++ code in Python. The Python part inspected the chip's specs and outputted all the instruction variations. Going back and forth being generator and generated code was a pain when debugging. I'd prefer a nicer workflow.

The m68k's opcodes are coded on 16 bits so there are *only* 65536 possible instructions. I could generate each one of them and store them in some sort of jump table. Pseudo-code:

```
void m68k()
{
 for (int i = 0; i < 0x10000; ++)
   instructions[i] = generate(i);

 while (1)
   intructions[memory[pc++]]();
}
```

The `generate(opcode)` is the tricky part. There, I would need to parse the bit pattern of `opcode` and return a function executing this particular variation, with constant operands and no branching. In C# I would use code generation. In Javascript, I could use some neat tricks with closures. How to do that in plain C? I guess I'll learn function pointers.

I looked at other m68k implementations and Musashi, a reference emulator used in Mame, seems to take this approach (see [`build_opcode_table()`](https://github.com/kstenerud/Musashi/blob/ad677ba25e887a7cbaa6f5419d6e2f00cb60a1f5/m68kdasm.c#L3197)). I'll start experimenting.

## 10/26/2016

I wanted to start writing a new console emulator for some time now. There's still work to be done on my other projects -- could add some fun features to [Boyo.js](https://github.com/merwaaan/boyo.js) and [mr.systel](https://github.com/merwaaan/mr.system) is far from being functional -- but I need something fresh. I chose to try and emulate the Sega Genesis/Megadrive since it was my first game console and it's a bit special to me.

Technically, the Genesis is reknowned for its performance compared to the other game systems of its time so I will have to pay particular attention to this aspect (Boyo.js was not on the fast side). I won't use Javascript to emulate the chips, it was too much trouble last time (no byte type, no unsigned type, needed to mask every computation by 0xFF...). However, I'd love for this project to be rendered in the browser with a nice UI for the debugger.

My initial choice is to write the emulator modules in C and to compile them to JS with emscripten in order to benefit from ASM.js optimizations. The user interface and rendering will be done in Javascript. We'll see how this go.
