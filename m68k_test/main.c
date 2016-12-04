#include "bit_utils.h"
#include "instructions_logic.h"

int main()
{
    MU_RUN_SUITE(test_suite_bit_utils);
    MU_RUN_SUITE(test_suite_instructions_logic);

    MU_REPORT();

    getchar();
    return 0;
}