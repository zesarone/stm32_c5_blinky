#include <stdint.h>
#include "segment_display.h"

#define MAX_ITERATIONS 10000

static double pi_value = 0.0;
static uint32_t iterations = 0;
static uint8_t precision_level = 0;

static double calculate_pi_leibniz(uint32_t max_iter) {
    double pi = 0.0;
    int sign = 1;

    for (uint32_t i = 0; i < max_iter; i++) {
        pi += sign * 4.0 / (2 * i + 1);
        sign = -sign;
    }

    return pi;
}

void state_2_run(void) {
    iterations++;

    if (iterations > MAX_ITERATIONS) {
        iterations = 1;
        precision_level++;
        if (precision_level > 3) {
            precision_level = 0;
        }
    }

    pi_value = calculate_pi_leibniz(iterations);

    // Display pi with current precision level
    // precision_level 0: show integer part (3)
    // precision_level 1: show 1 decimal (3.1)
    // precision_level 2: show 2 decimals (3.14)
    // precision_level 3: show 3 decimals (3.141)

    uint16_t display_value;
    switch (precision_level) {
        case 0:
            display_value = (uint16_t)pi_value; // 3
            break;
        case 1: {
            int int_part = (int)pi_value;
            int dec_part = (int)((pi_value - int_part) * 10);
            display_value = int_part * 10 + dec_part; // 31 (represents 3.1)
            break;
        }
        case 2: {
            int int_part = (int)pi_value;
            int dec_part = (int)((pi_value - int_part) * 100);
            display_value = int_part * 100 + dec_part; // 314 (represents 3.14)
            break;
        }
        case 3: {
            int int_part = (int)pi_value;
            int dec_part = (int)((pi_value - int_part) * 1000);
            display_value = int_part * 1000 + dec_part; // 3141 (represents 3.141)
            break;
        }
        default:
            display_value = (uint16_t)pi_value;
            break;
    }

    segment_display_show_number(display_value);
}