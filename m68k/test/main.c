#include "bit_utils.h"
#include "addressing_modes.h"

int main()
{
    MU_RUN_TEST(test_parse_bin);

    MU_RUN_SUITE(test_suite_addressing_modes);

    MU_REPORT();

    return 0;
}
