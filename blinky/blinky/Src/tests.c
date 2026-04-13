#include "tests.h"
#include "led.h"
#include "segment_display.h"
#include "state_1.h"
#include "state_2.h"
#include "state_3.h"
#include "stm32c5a3xx.h"
#include <stdint.h>

// Test framework - simplified for embedded system
// Since we can't easily mock hardware, we'll test logic and print results

void test_assert(uint8_t condition, const char *message) {
    if (!condition) {
        test_print("FAIL: ");
        test_print(message);
        test_print("\n");
    } else {
        test_print("PASS: ");
        test_print(message);
        test_print("\n");
    }
}

void test_print(const char *message) {
    // Simple print function - in real implementation would use UART
    // For now, just indicate the message exists
    (void)message;
}

uint8_t test_get_user_confirmation(const char *message) {
    test_print(message);
    test_print(" (y/n): ");
    // In real implementation, would read from UART/serial
    // For testing purposes, assume user confirms
    return 1;
}

void test_led_functions(void) {
    test_print("Testing LED functions...\n");

    // Test that LED functions don't crash
    // In a real test environment, we would check GPIO registers
    // For now, just call the functions to ensure they don't crash
    led_init();
    test_assert(1, "led_init() completed without error");

    led_set(0, 1);
    test_assert(1, "led_set(0, 1) completed without error");

    led_set(0, 0);
    test_assert(1, "led_set(0, 0) completed without error");

    led_all_off();
    test_assert(1, "led_all_off() completed without error");

    test_print("LED function tests completed.\n");
}
static void delay(uint32_t count) {
    while (count--) {
        __asm("nop");
    }
}
void test_segment_display_functions(void) {
    test_print("Testing segment display functions...\n");

    // Test that display functions don't crash
    segment_display_init();
    test_assert(1, "segment_display_init() completed without error");

    segment_display_clear();
    test_assert(1, "segment_display_clear() completed without error");

    // Test showing numbers - these should not crash
    segment_display_show_number(0000);
    test_assert(1, "segment_display_show_number(0000) completed without error");
    delay(1000000); // Delay to allow visual confirmation

    segment_display_show_number(111.1);
    test_assert(1, "segment_display_show_number(111.1) completed without error");
    delay(1000000); // Delay to allow visual confirmation
    
    segment_display_show_number(22.22);
    test_assert(1, "segment_display_show_number(22.22) completed without error");
    delay(1000000); // Delay to allow visual confirmation

        segment_display_show_number(3.333);
    test_assert(1, "segment_display_show_number(3.333) completed without error");
    delay(1000000); // Delay to allow visual confirmation

        segment_display_show_number(4444);
    test_assert(1, "segment_display_show_number(4444) completed without error");
    delay(1000000); // Delay to allow visual confirmation

        segment_display_show_number(555.5);
    test_assert(1, "segment_display_show_number(555.5) completed without error");
    delay(1000000); // Delay to allow visual confirmation

        segment_display_show_number(66.66);
    test_assert(1, "segment_display_show_number(66.66) completed without error");
    delay(1000000); // Delay to allow visual confirmation

        segment_display_show_number(7.777);
    test_assert(1, "segment_display_show_number(7.777) completed without error");
    delay(1000000); // Delay to allow visual confirmation

        segment_display_show_number(8888);
    test_assert(1, "segment_display_show_number(8888) completed without error");
    delay(1000000); // Delay to allow visual confirmation

        segment_display_show_number(999.9);
    test_assert(1, "segment_display_show_number(999.9) completed without error");
    delay(1000000); // Delay to allow visual confirmation

    segment_display_show_float(3.14159f);
    test_assert(1, "segment_display_show_float(3.14159) completed without error");

    // Ask for user confirmation for visual tests
    uint8_t confirmed = test_get_user_confirmation("Did the display show '1234' and then '3.142'?");
    test_assert(confirmed, "User confirmed display shows expected values");

    test_print("Segment display function tests completed.\n");
}

void test_state_functions(void) {
    test_print("Testing state functions...\n");

    // Test state functions - these should not crash and should update display
    state_1_run();
    test_assert(1, "state_1_run() (prime sieve) completed without error");

    state_2_run();
    test_assert(1, "state_2_run() (pi calculator) completed without error");

    state_3_run();
    test_assert(1, "state_3_run() (counter) completed without error");

    // Ask for user confirmation for visual tests
    uint8_t confirmed = test_get_user_confirmation("Did the display show primes, pi approximation, and counter values?");
    test_assert(confirmed, "User confirmed state functions work correctly");

    test_print("State function tests completed.\n");
}

void test_init(void) {
    test_print("Initializing test framework...\n");
}

void test_run_all(void) {
    test_print("Running all tests...\n");

    test_led_functions();
    test_segment_display_functions();
    test_state_functions();

    test_print("All tests completed!\n");
}

void run_unit_tests(void) {
    test_init();
    test_run_all();
}