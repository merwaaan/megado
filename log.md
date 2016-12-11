# Logs

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

I wanted to start writing a new console emulator for some time now. There's still work to be done on my other projects -- could add some fun features to [Boyo.js](https://github.com/merwaaan/boyo.js) and [mr.genesis](https://github.com/merwaaan/mr.system) is far from being functional -- but I need something fresh. I chose to try and emulate the Sega Genesis/Megadrive since it was my first game console and it's a bit special to me.

Technically, the Genesis is reknowned for its performance compared to the other game systems of its time so I will have to pay particular attention to this aspect (Boyo.js was not on the fast side). I won't use Javascript to emulate the chips, it was too much trouble last time (no byte type, no unsigned type, needed to mask every computation by 0xFF...). However, I'd love for this project to be rendered in the browser with a nice UI for the debugger.

My initial choice is to write the emulator modules in C and to compile them to JS with emscripten in order to benefit from ASM.js optimizations. The user interface and rendering will be done in Javascript. We'll see how this go.
