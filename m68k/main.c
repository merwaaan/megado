#include "m68k.h"

int main()
{
	M68k* m = m68k_init();

	m68k_free(m);

	return 0;
}
