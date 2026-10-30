#include <print>

#include "app.h"

using namespace std;

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        println(stderr, "Usage: {} ROM", argv[0]);
        return 1;
    }

    App app;

    app.load_rom_file(argv[1]);

    app.run();

    return 0;
}
