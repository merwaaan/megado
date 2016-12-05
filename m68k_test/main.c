#include "bit_utils.h"
#include "instructions_bit.h"
#include "instructions_logic.h"
#include "instructions_shift.h"
#include "instructions_transfer.h"

int main()
{
    MU_RUN_SUITE(test_suite_bit_utils);
    MU_RUN_SUITE(test_suite_instructions_bit);
    //MU_RUN_SUITE(test_suite_instructions_logic);
    MU_RUN_SUITE(test_suite_instructions_shift);
    MU_RUN_SUITE(test_suite_instructions_transfer);

    MU_REPORT();

    getchar();
    return 0;
}