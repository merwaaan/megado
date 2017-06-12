# Bugs

- Sunset Riders: the palettes become greenish on the menu screen
                 (switches back to normal after a few seconds in-game)

- Columns: the sega boot animation only appears in Release builds (Visual Studio specific)
           (no SEGA logo with clang either in release and debug)

- Alex Kidd in the Enchanted Castle: stuck on the SEGA logo. The program seems to be stuck in a loop,
waiting for the "v-blank in progress" bit in the VDP status to be set. The issue here is that this
bit is set and then cleared in the same scanline but we do not execute instructions while drawing
scanlines. Is this a hint that we should draw pixels and execute instructions in parallel?


# Games that seem to wait for the Z80:

- Daffy Duck in Hollywood @1D7BC8
- The Addam's Family @43C0E
