#pragma once

// References
// https://emu-docs.org/Genesis/gen-hw.txt (Section 3.2)
// https://wiki.megadrive.org/index.php?title=Reading_MD3_Joypad

typedef enum
{
    // The enum value corresponds to the mask for each input in
    // the multiplexed word that holds the controller's state.
    Up = 0x101, 
    Down = 0x202, // Up and Down can be read in both bytes
    Left = 0x4,
    Right = 0x8,
    ButtonB = 0x10,
    ButtonC = 0x20,
    ButtonA = 0x1000,
    Start = 0x2000
} JoypadButton;

typedef struct Joypad {
    // The low byte contains the first half of the controller's state (Up, Down, Left, Right, B, C).
    // The high byte contains the second half of the controller's state (Up, Down, A, Start).
    //
    // Bit 7 and 6 can be written to.
    // Bit 7 has no effect.
    // Bit 6 controls which of the high or low byte is accessed when reading the controller's port.
    // 
    // A cleared bit means that the button is pressed!
    uint16_t buttons;
} Joypad;

Joypad* joypad_make();
void joypad_free(Joypad*);

uint8_t joypad_read(Joypad*);
void joypad_write(Joypad*, uint8_t);

void joypad_press(Joypad*, JoypadButton);
void joypad_release(Joypad*, JoypadButton);
