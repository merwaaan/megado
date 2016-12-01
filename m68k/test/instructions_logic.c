#include "unity.h"

#include "../instructions_logic.h"
#include "../m68k.h"

M68k* cpu = NULL;

uint16_t parse_bin(char* bin)
{
    uint16_t value;

    int i = 0;
    while (bin[i] != '\0')
        if (bin[i] == '1')
            value = (value << 1) + 1;
        else if (bin[i] == '0')
            value <<= 1;
        else
            bin++;

    return value;
}

void SetUp()
{
    cpu = m68k_init();
}

void tearDown()
{
    m68k_free(cpu);
}

void xxx()
{
    m68k_execute(cpu, parse_bin("1100 001 0 00 000 0100")); // AND D1, D4
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(xxx);
    return UNITY_END();
}
