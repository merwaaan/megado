# Logs

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
