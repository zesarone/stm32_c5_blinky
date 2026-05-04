#include <stdbool.h>
#include "segment_display.h"

#define DIGIT_BLANK_VALUE     0xFFu
#define DIGIT_PATTERN_BLANK   0x00u
#define DIGIT_PATTERN_C       0x39u
#define DISPLAY_REFRESH_CYCLES 125u
#define DISPLAY_DELAY_CYCLES  1000u

typedef struct {
    GPIO_TypeDef *port;
    uint32_t pin_mask;
} PinBinding;

static const uint8_t digit_patterns[10] = {
    SEG_0,
    SEG_1,
    SEG_2,
    SEG_3,
    SEG_4,
    SEG_5,
    SEG_6,
    SEG_7,
    SEG_8,
    SEG_9
};

static const PinBinding segment_bindings[8] = {
    { SEG_A_PORT, (1u << SEG_A_PIN) },
    { SEG_B_PORT, (1u << SEG_B_PIN) },
    { SEG_C_PORT, (1u << SEG_C_PIN) },
    { SEG_D_PORT, (1u << SEG_D_PIN) },
    { SEG_E_PORT, (1u << SEG_E_PIN) },
    { SEG_F_PORT, (1u << SEG_F_PIN) },
    { SEG_G_PORT, (1u << SEG_G_PIN) },
    { SEG_DP_PORT, (1u << SEG_DP_PIN) }
};

static const PinBinding digit_bindings[4] = {
    { DIGIT1_PORT, (1u << DIGIT1_PIN) },
    { DIGIT2_PORT, (1u << DIGIT2_PIN) },
    { DIGIT3_PORT, (1u << DIGIT3_PIN) },
    { DIGIT4_PORT, (1u << DIGIT4_PIN) }
};

static void short_delay(void)
{
    for (volatile uint32_t i = 0u; i < DISPLAY_DELAY_CYCLES; ++i) {
        __asm("nop");
    }
}

static void clear_digits(uint8_t digits[4])
{
    for (uint8_t index = 0u; index < DIGIT_COUNT; ++index) {
        digits[index] = DIGIT_BLANK_VALUE;
    }
}

static uint32_t pow10_u32(uint8_t exponent)
{
    uint32_t result = 1u;

    while (exponent-- != 0u) {
        result *= 10u;
    }

    return result;
}

static uint8_t count_digits_u32(uint32_t value)
{
    uint8_t count = 1u;

    while (value >= 10u) {
        value /= 10u;
        count++;
    }

    return count;
}

static void display_all_off(void)
{
    for (uint8_t index = 0u; index < 8u; ++index) {
        segment_bindings[index].port->BSRR = segment_bindings[index].pin_mask << 16u;
    }

    for (uint8_t index = 0u; index < DIGIT_COUNT; ++index) {
        digit_bindings[index].port->BSRR = digit_bindings[index].pin_mask;
    }
}

static void display_pattern(uint8_t digit_index, uint8_t pattern, bool enable_dp)
{
    display_all_off();

    for (uint8_t segment = 0u; segment < 7u; ++segment) {
        if (((pattern >> segment) & 1u) != 0u) {
            segment_bindings[segment].port->BSRR = segment_bindings[segment].pin_mask;
        }
    }

    if (enable_dp) {
        segment_bindings[7].port->BSRR = segment_bindings[7].pin_mask;
    }

    digit_bindings[digit_index].port->BSRR = digit_bindings[digit_index].pin_mask << 16u;
}

static void convert_digits_to_patterns(const uint8_t digits[4], uint8_t patterns[4])
{
    for (uint8_t index = 0u; index < DIGIT_COUNT; ++index) {
        if (digits[index] == DIGIT_BLANK_VALUE) {
            patterns[index] = DIGIT_PATTERN_BLANK;
        } else {
            patterns[index] = digit_patterns[digits[index]];
        }
    }
}

static void render_patterns_mask_cycles(const uint8_t patterns[4], uint8_t decimal_point_mask, uint32_t refresh_cycles)
{
    for (uint32_t cycle = 0u; cycle < refresh_cycles; ++cycle) {
        for (uint8_t index = 0u; index < DIGIT_COUNT; ++index) {
            uint8_t enable_dp = ((decimal_point_mask >> index) & 1u) != 0u;

            display_pattern(index, patterns[index], enable_dp != 0u);
            short_delay();
        }
    }

    display_all_off();
}

static void render_patterns_cycles(const uint8_t patterns[4], int8_t decimal_point_index, uint32_t refresh_cycles)
{
    uint8_t decimal_point_mask = 0u;

    if ((decimal_point_index >= 0) && (decimal_point_index < (int8_t)DIGIT_COUNT)) {
        decimal_point_mask = (uint8_t)(1u << (uint8_t)decimal_point_index);
    }

    render_patterns_mask_cycles(patterns, decimal_point_mask, refresh_cycles);
}

static void render_patterns(const uint8_t patterns[4], int8_t decimal_point_index)
{
    render_patterns_cycles(patterns, decimal_point_index, DISPLAY_REFRESH_CYCLES);
}

static void format_unsigned_digits(uint16_t number, uint8_t digits[4])
{
    clear_digits(digits);

    if (number == 0u) {
        digits[3] = 0u;
        return;
    }

    int8_t position = 3;
    while ((number != 0u) && (position >= 0)) {
        digits[position] = (uint8_t)(number % 10u);
        number /= 10u;
        position--;
    }
}

static void format_float_digits(float value, uint8_t digits[4], int8_t *decimal_point_index)
{
    clear_digits(digits);

    if (value < 0.0f) {
        value = 0.0f;
    }

    if (value >= 10000.0f) {
        value = 9999.0f;
    }

    uint32_t integer_part = (uint32_t)value;
    float fractional_part = value - (float)integer_part;

    if ((fractional_part < 0.00001f) || (integer_part >= 1000u)) {
        *decimal_point_index = -1;
        format_unsigned_digits((uint16_t)integer_part, digits);
        return;
    }

    uint8_t decimals;
    if (integer_part < 10u) {
        decimals = 3u;
    } else if (integer_part < 100u) {
        decimals = 2u;
    } else {
        decimals = 1u;
    }

    uint32_t scale = pow10_u32(decimals);
    uint32_t scaled_value = (uint32_t)(value * (float)scale + 0.5f);

    while ((decimals > 1u) && ((scaled_value % 10u) == 0u)) {
        scaled_value /= 10u;
        decimals--;
    }

    uint32_t divider = pow10_u32(decimals);
    uint32_t display_integer = scaled_value / divider;
    uint32_t display_fraction = scaled_value % divider;

    if (display_integer >= 1000u) {
        *decimal_point_index = -1;
        format_unsigned_digits((uint16_t)display_integer, digits);
        return;
    }

    uint8_t integer_digits = count_digits_u32(display_integer);
    int8_t position = 3;

    for (uint8_t index = 0u; index < decimals; ++index) {
        digits[position] = (uint8_t)(display_fraction % 10u);
        display_fraction /= 10u;
        position--;
    }

    for (uint8_t index = 0u; index < integer_digits; ++index) {
        digits[position] = (uint8_t)(display_integer % 10u);
        display_integer /= 10u;
        position--;
    }

    *decimal_point_index = (int8_t)(3 - decimals - (integer_digits - 1u));
}

static void format_temperature_digits(float temperature, uint8_t patterns[4], int8_t *decimal_point_index)
{
    uint8_t digits[4];
    int32_t scaled_temperature;

    clear_digits(digits);

    if (temperature >= 0.0f) {
        scaled_temperature = (int32_t)(temperature * 10.0f + 0.5f);
    } else {
        scaled_temperature = (int32_t)(temperature * 10.0f - 0.5f);
    }

    if (scaled_temperature < -999) {
        scaled_temperature = -999;
    }

    if (scaled_temperature > 999) {
        scaled_temperature = 999;
    }

    if (scaled_temperature < 0) {
        scaled_temperature = -scaled_temperature;
    }

    digits[0] = (scaled_temperature >= 100) ? (uint8_t)((scaled_temperature / 100) % 10) : DIGIT_BLANK_VALUE;
    digits[1] = (uint8_t)((scaled_temperature / 10) % 10);
    digits[2] = (uint8_t)(scaled_temperature % 10);

    convert_digits_to_patterns(digits, patterns);
    patterns[3] = DIGIT_PATTERN_C;
    *decimal_point_index = 1;
}

void segment_display_init(void)
{
    display_all_off();
}

void segment_display_clear(void)
{
    display_all_off();
}

void segment_display_show_number(uint16_t number)
{
    uint8_t digits[4];
    uint8_t patterns[4];

    format_unsigned_digits(number, digits);
    convert_digits_to_patterns(digits, patterns);
    render_patterns(patterns, -1);
}

void segment_display_show_number_once(uint16_t number)
{
    uint8_t digits[4];
    uint8_t patterns[4];

    format_unsigned_digits(number, digits);
    convert_digits_to_patterns(digits, patterns);
    render_patterns_cycles(patterns, -1, 1u);
}

void segment_display_scan_number_step(uint16_t number)
{
    static uint8_t current_digit = 0u;
    uint8_t digits[4];
    uint8_t patterns[4];

    format_unsigned_digits(number, digits);
    convert_digits_to_patterns(digits, patterns);

    display_pattern(current_digit, patterns[current_digit], false);
    short_delay();
    display_all_off();

    current_digit++;
    if (current_digit >= DIGIT_COUNT) {
        current_digit = 0u;
    }
}

void segment_display_show_float(float value)
{
    uint8_t digits[4];
    uint8_t patterns[4];
    int8_t decimal_point_index;

    format_float_digits(value, digits, &decimal_point_index);
    convert_digits_to_patterns(digits, patterns);
    render_patterns(patterns, decimal_point_index);
}

void segment_display_show_digit(uint8_t digit, uint8_t position, uint8_t dp)
{
    uint8_t patterns[4] = {
        DIGIT_PATTERN_BLANK,
        DIGIT_PATTERN_BLANK,
        DIGIT_PATTERN_BLANK,
        DIGIT_PATTERN_BLANK
    };

    if ((position >= DIGIT_COUNT) || (digit > 9u)) {
        display_all_off();
        return;
    }

    patterns[position] = digit_patterns[digit];
    render_patterns(patterns, (dp != 0u) ? (int8_t)position : -1);
}

void segment_display_show_raw_once(const uint8_t patterns[4], uint8_t decimal_point_mask)
{
    render_patterns_mask_cycles(patterns, decimal_point_mask, 1u);
}

void segment_display_show_temperature(float temperature)
{
    uint8_t patterns[4];
    int8_t decimal_point_index;

    format_temperature_digits(temperature, patterns, &decimal_point_index);
    render_patterns(patterns, decimal_point_index);
}
