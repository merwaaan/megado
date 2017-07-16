# Game-specific bugs

- Columns:
  - score overflows its box (looks like the most significant digit is a repeat
    of the least-significant one, when it should be zero; the bogus digit is
    written to the VDP @ 63AA)

- Alex Kidd in the Enchanted Castle:
  - Planes are empty during the intro and in-game (only works in the korean version)

- The Simpsons - Bart's Nightmare: 
  - Bart's sprite is not visible

- Spiderman and Venom - Maximum Carnage / Separation Anxiety: Spiderman is not animated.

- Spiderman vs The Kingpin:
  - pressing start on the intro screen make the screen flash indefinitely, going faster with
  each key press.

- Splatterhouse 2: waits for a DMA transfer (bit 1 of VDP status) but we do those in a single step
so the flag is always cleared.

- Quackshot: Donald's sprite is scrambled

- Landstalker: waits for $FF0F8B @B6E

- Sonic & Knuckles + Sonic 3: stuck in an infinite loop @C336

- Streets of Rage: character seems to die randomly; time over happens after a few seconds

- Street Fighter II: game reset just before going into the first stage

- Krusty: playable; but item sprite is garbled when in the holding slot

- Castlevania Bloodlines: in-game stage has sprite glitches; choosing the second character instantly
  glitches the whole screen.

- Toy Story: executes the content of the stack.

- Blades of Vengeance:
    - copyright screen partially wraps around the screen.
    - the menu screen does not respond.

- Chameleon Kid: character does not walk but looks in the correct direction.

- Ristar: goes to the next level when grabbing an enemy.

- Contra:
    - character fires his weapon backward!
	- no watery animation of the game logo
	- the intro sequence of the first level is a mess

- Zombies Ate My Neighbors:
	- the film roll on the character select screen must be transparent (how is this done?)
	- plane scrolling is visible at the right of the screen 

- Tale Spin: wait for @FF000E to be written to (during an interrupt?)

- Clue: main menu not responding

# Games that seem to wait for the Z80:

- Daffy Duck in Hollywood @1D7BC8
- The Addam's Family @43C0E
- Lost Vikings: waiting for $A01000 to equal zero @FD888 (eerily similar to Addam's Family)
- Pitfall
- Wolverine Adamantium Rage
