#include <stdint.h>
#include <stdlib.h>

#include "joypad.h"

Joypad* joypad_make()
{
    Joypad* j = calloc(1, sizeof(Joypad));
    j->buttons = 0x333F; // No buttons pressed
    return j;
}

void joypad_free(Joypad* joypad)
{
    free(joypad);
}

uint8_t joypad_read(Joypad* joypad)
{
    // Return either the high or low byte depending on bit 6
    return joypad->buttons & 0x40 ? joypad->buttons : joypad->buttons >> 8;
}

void joypad_write(Joypad* joypad, uint8_t value)
{
    // Only write bit 6 (in both the low and high bytes)
    joypad->buttons = (joypad->buttons & 0xBFBF) | ((value | (value << 8)) & 0x4040);
}

void joypad_press(Joypad* joypad, JoypadButton button)
{
    joypad->buttons &= ~button;
}

void joypad_release(Joypad* joypad, JoypadButton button)
{
    joypad->buttons |= button;
}
