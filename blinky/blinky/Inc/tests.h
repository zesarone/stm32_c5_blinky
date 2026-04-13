#ifndef TESTS_H
#define TESTS_H

#include <stdint.h>

// Test framework functions
void test_init(void);
void test_run_all(void);
void test_led_functions(void);
void test_segment_display_functions(void);
void test_state_functions(void);

// Helper functions
void test_assert(uint8_t condition, const char *message);
void test_print(const char *message);
uint8_t test_get_user_confirmation(const char *message);

// Test runner - call this to run all tests
void run_unit_tests(void);

#endif // TESTS_H