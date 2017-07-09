# Game-specific bugs

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
  - Planes are empty during the intro and in-game (only works in the korean version)
  - The sega logo's gradient is not animated

- Shadowrun: the games becomes completely glitched once the in-game menu is opened.

- The Simpsons - Bart's Nightmare: the planes have an invalid size (64*128). This occurs on
the menu screen and then in-game, which leads to a buffer overflow.

- Spiderman and Venom - Maximum Carnage / Separation Anxiety: Spiderman is not animated.

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

- Streets of Rage: no UI in-game; character seems to die randomly; time over
  happens after a few seconds

- Street Fighter II: game reset just before going into the first stage

- Krusty: playable; but item sprite is garbled when in the holding slot

- Castlevania Bloodlines: palette is offset to the blue most of the time;
  in-game stage has sprite glitches; choosing the second character instantly
  glitches the whole screen.

- Toy Story: executes the content of the stack.

- Blades of Vengeance:
    - copyright screen partially wraps around the screen.
    - the menu screen does not respond.

- California Games: the menu screen does not respond.

- Chameleon Kid: character does not walk but looks in the correct direction.

- Ristar: character goes below the ground.

- Contra:
    - no UI (Window plane not shown).
    - character fires his weapon backward!

- Castle of Illusion: Mickey's jumps are too short to pass the first step.

# Games that seem to wait for the Z80:

- Daffy Duck in Hollywood @1D7BC8
- The Addam's Family @43C0E
- Lost Vikings: waiting for $A01000 to equal zero @FD888 (eerily similar to Addam's Family)
- Spirou: seems to be waiting for the Z80 to execute something
- Pitfall
