#include <stdint.h>
#include "segment_display.h"

#define MAX_ITERATIONS 1000000
#define SCROLL_WINDOW_SIZE 4
#define SCROLL_MAX_POSITION 12

static double pi_value = 0.0;
static uint32_t iterations = 0;
static uint8_t precision_level = 0;
static uint8_t scroll_mode = 0;
static uint8_t scroll_position = 0;

static double calculate_pi_leibniz(uint32_t max_iter) {
    double pi = 0.0;
    int sign = 1;

    for (uint32_t i = 0; i < max_iter; i++) {
        pi += sign * 4.0 / (2 * i + 1);
        sign = -sign;
    }

    return pi;
}

static float pi_display_value(double pi, uint8_t precision) {
    uint32_t scale = 1u;
    for (uint8_t i = 0; i < precision; ++i) {
        scale *= 10u;
    }

    uint32_t rounded = (uint32_t)(pi * (double)scale + 0.5);
    return (float)rounded / (float)scale;
}

static float pi_scroll_value(uint8_t position) {
    static const char pi_digits[] = "3141592653589793238462643383279502884197169399375105820974944592";
    uint32_t value = 0u;

    for (uint8_t i = 0u; i < SCROLL_WINDOW_SIZE; ++i) {
        uint8_t digit = (uint8_t)(pi_digits[position + i] - '0');
        value = value * 10u + digit;
    }

    return (float)value / 1000.0f;
}

void state_2_run(void) {
    iterations++;

    pi_value = calculate_pi_leibniz(iterations);

    if (!scroll_mode && pi_value >= 3.141) {
        scroll_mode = 1;
        scroll_position = 0;
    }

    if (scroll_mode) {
        float display_value = pi_scroll_value(scroll_position);
        segment_display_show_float(display_value);

        static uint16_t scroll_counter = 0u;
        scroll_counter++;
        if (scroll_counter >= 1000u) {
            scroll_counter = 0u;
            scroll_position++;
            if (scroll_position > SCROLL_MAX_POSITION) {
                scroll_position = 0u;
            }
        }
    } else {
        if (iterations % 1000u == 0u && precision_level < 3u) {
            precision_level++;
        }

        float display_value = pi_display_value(pi_value, precision_level);
        segment_display_show_float(display_value);
    }

    if (iterations >= MAX_ITERATIONS) {
        iterations = 0u;
        precision_level = 0u;
        scroll_mode = 0u;
        scroll_position = 0u;
    }
}
