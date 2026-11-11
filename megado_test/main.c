#include <greatest.h>

SUITE_EXTERN(test_suite_addressing_modes);
SUITE_EXTERN(test_suite_bit_utils);
SUITE_EXTERN(test_suite_instructions_arithmetic);
SUITE_EXTERN(test_suite_instructions_bcd);
SUITE_EXTERN(test_suite_instructions_bit);
SUITE_EXTERN(test_suite_instructions_logic);
SUITE_EXTERN(test_suite_instructions_shift);
SUITE_EXTERN(test_suite_instructions_transfer);

GREATEST_MAIN_DEFS();

int main(int argc, char** argv)
{
    GREATEST_MAIN_BEGIN();

    RUN_SUITE(test_suite_bit_utils);
    RUN_SUITE(test_suite_addressing_modes);
    RUN_SUITE(test_suite_instructions_arithmetic);
    RUN_SUITE(test_suite_instructions_bcd);
    RUN_SUITE(test_suite_instructions_bit);
    RUN_SUITE(test_suite_instructions_logic);
    RUN_SUITE(test_suite_instructions_shift);
    RUN_SUITE(test_suite_instructions_transfer);

    GREATEST_MAIN_END();
}
