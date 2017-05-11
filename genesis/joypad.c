#include <stdint.h>
#include <stdlib.h>

#include "joypad.h"

// https://emu-docs.org/Genesis/gen-hw.txt (Section 3.2)

Joypad* joypad_make()
{
    return calloc(1, sizeof(Joypad));
}

void joypad_free(Joypad* joypad)
{
    free(joypad);
}

uint8_t joypad_read(Joypad* joypad)
{
    // Return either the high or low byte depending on bit 6
    return joypad->buttons & 0x40 ? joypad->buttons >> 8 : joypad->buttons;
}

void joypad_write(Joypad* joypad, uint8_t value)
{
    // Only write bit 6 and 7 (in both low and high bytes)
    joypad->buttons = (joypad->buttons & 0x3F3F) | ((value | (value << 8)) & 0xC0C0);
}

void joypad_press(Joypad* joypad, JoypadButton button)
{
    joypad->buttons |= button;
}

void joypad_release(Joypad* joypad, JoypadButton button)
{
    joypad->buttons &= ~button;
}
