# Bugs

- Sunset Riders: the palettes become greenish on the menu screen
                 (switches back to normal after a few seconds in-game).

- Tiny Toon Adventures - Acme All-Stars: the palettes become greenish on the menu screen.

- Tiny Toon Adventures - Buster's Hidden Treasure: the palettes become greenish in-game.

- Columns: the sega boot animation only appears in Release builds (Visual Studio specific)
           (no SEGA logo with clang either in release and debug).

- Alex Kidd in the Enchanted Castle: Planes are empty during the intro and in-game.

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

# Games that seem to wait for the Z80:

- Daffy Duck in Hollywood @1D7BC8
- The Addam's Family @43C0E
