# Game-specific bugs

- Columns:
  - score overflows its box (looks like the most significant digit is a repeat
    of the least-significant one, when it should be zero; the bogus digit is
    written to the VDP @ 63AA)

- The Simpsons - Bart's Nightmare: 
  - Bart's sprite is not visible
  
- Splatterhouse 2: waits for a DMA transfer (bit 1 of VDP status) but we do those in a single step
so the flag is always cleared.

- Quackshot: The plane's sprite is scrambled

- Landstalker: waits for $FF0F8B @B6E

- Streets of Rage: character seems to die randomly; time over happens after a few seconds

- Street Fighter II: game reset just before going into the first stage

- Krusty: playable; but item sprite is garbled when in the holding slot

- Castlevania Bloodlines: in-game stage has sprite glitches; choosing the second character instantly
  glitches the whole screen.

- Toy Story:
  - graphics seem glitched
  - frozen during intro

- Blades of Vengeance:
    - copyright screen blitted twice with an offset (might be the Window plane that should not be enabled but is still drawn)
    - the menu screen does not respond.
	
- Ristar: goes to the next level when grabbing an enemy.

- Contra:
    - character fires his weapon backward!
	- no watery animation of the game logo
	- the intro sequence of the first level is a mess

- Zombies Ate My Neighbors:
	- the film roll on the character select screen must be transparent (how is this done?)
	- plane scrolling is visible at the right of the screen
	
- Another World: scanlines are drawn out. This game might very sensible to timing accuracy since
it implements its own polygon rasterizer.

- Jurassic Park: infinite loop in the first level.

- Jurassic Park 2: executes invalid opcodes.

- Last Action Hero: overwrites a big section of ROM.

- Carmen Sandiego (World/Time): some columns of graphics are glitched.

- We're back!: requires ILLEGAL instruction.

- Warsong: requires DMA copy.

# Games that seem to wait for the Z80:

- Daffy Duck in Hollywood @1D7BC8
- The Addam's Family @43C0E
- Lost Vikings: waiting for $A01000 to equal zero @FD888 (eerily similar to Addam's Family)
- Pitfall
- Wolverine Adamantium Rage
- Alien 3
- Chuck Rock
- Hook
- Batman Forever
- Mr Nutz
