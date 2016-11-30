#include "globals.h"

extern uint16_t *memory;

uint16_t bit(int x, int position)
{
	return (x & (1 << position)) >> position;
}

uint16_t fragment(int x, int start, int end)
{
	return (x & ~(0xFFFF << start)) >> end;
}
