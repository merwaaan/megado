#include "genesis.h";

int main()
{
    Genesis* g = genesis_make();
    genesis_load_rom(g, "xxx.md");

    

    genesis_free(g);

    return 0;
}
