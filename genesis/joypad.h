#pragma once

typedef enum
{
    Up = 0x101, // Up and Down can be read in both parts
    Down = 0x202,
    Left = 0x4,
    Right = 0x8,
    ButtonB = 0x10,
    ButtonC = 0x20,
    ButtonA = 0x1000,
    Start = 0x2000
} JoypadButton;

typedef struct Joypad {

    // Word containing all the buttons.
    // Depending on bit 6, the high or low byte will be read.
    uint16_t buttons;
} Joypad;

Joypad* joypad_make();
void joypad_free(Joypad*);

uint8_t joypad_read(Joypad*);
void joypad_write(Joypad*, uint8_t);

void joypad_press(Joypad*, JoypadButton);
void joypad_release(Joypad*, JoypadButton);
