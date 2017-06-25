# Bugs

- Sunset Riders: the palettes become greenish on the menu screen
                 (switches back to normal after a few seconds in-game).

- Tiny Toon Adventures - Acme All-Stars: the palettes become greenish on the menu screen.

- Tiny Toon Adventures - Buster's Hidden Treasure: the palettes become greenish in-game.

- Columns:
  - the sega boot animation only appears in Release builds (Visual Studio specific)
    (no SEGA logo with clang either in release and debug).
  - score overflows its box (looks like the most significant digit is a repeat
    of the least-significant one, when it should be zero; the bogus digit is
    written to the VDP @ 63AA)

- Alex Kidd in the Enchanted Castle:
  - Planes are empty during the intro and in-game.
  - The sega logo's gradient is not animated

- Shadowrun: the games becomes completely glitched once the in-game menu is opened.

- The Simpsons - Bart's Nightmare: the planes have an invalid size (64*128). This occurs on
the menu screen and then in-game, which leads to a buffer overflow.

- Spiderman and Venom - Maximum Carnage: the palette is entirely white on the logo screens,
entirely black in-game.

- Spiderman and Venom - Separation Anxiety: Spiderman is not animated (the exact same bug
seem to occur on the boot screen of Maximum Carnage).

- Spiderman vs The Kingpin:
  - the intro screen has corrupted rows of tiles.
  - pressing start on the intro screen make the screen flash indefinitely, going faster with
  each key press.

- Teenage Mutant Ninja Turtles - Tournament Fighters: the character slides on the floor and does
not respond to keypresses. Sometimes he flickers violently from left to right.

- Splatterhouse 2: waits for a DMA transfer (bit 1 of VDP status) but we do those in a single step
so the flag is always cleared.

- Quackshot: can start game and play, but something is very wrong with the
  display. The VRAM is corrupted because of an invalid DMA transfer with
  length 0 (spans the whole VRAM) and auto-increment 2 (explains the stripes).
  Another DMA transfer occured a few cycles before and decremented the length
  to zero. Possible that the program did not have enough time to setup the
  faulty DMA because of a timing issue?

- Landstalker: waits for $FF0F8B @B6E

- Sonic & Knuckles + Sonic 3: stuck in an infinite loop @C336

- Donald in Maui Mallard: SEGA logo cut; in-game background is wonky, but Donald
  sprite looks alright

- Streets of Rage: no UI in-game; character seems to die randomly; time over
  happens after a few seconds

- Street Fighter II: crashes on an invalid opcode after executing unknown
  opcodes; last good opcode seems to be around @83A8

- Krusty: playable; but item sprite is garbled when in the holding slot

# Games that seem to wait for the Z80:

- Daffy Duck in Hollywood @1D7BC8
- The Addam's Family @43C0E
- Lost Vikings: waiting for $A01000 to equal zero @FD888 (eerily similar to Addam's Family)
- Spirou: seems to be waiting for the Z80 to execute something

# Invalid generated instructions

- 0x017C: BCHG with Immediate value as destination (fetch/get/set functions are
  NULL, so segfault)
- 0x01FC: BSET with Immediate value as destination
- 0x19F1: MOV with Immediate value as destination
