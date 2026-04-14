#include <stdbool.h>
#include "segment_display.h"
#include "stm32c5a3xx.h"

#define DIGIT_BLANK 0xFF

static const uint8_t segment_map[10] = {
    0x3Fu, /* 0 */
    0x06u, /* 1 */
    0x5Bu, /* 2 */
    0x4Fu, /* 3 */
    0x66u, /* 4 */
    0x6Du, /* 5 */
    0x7Du, /* 6 */
    0x07u, /* 7 */
    0x7Fu, /* 8 */
    0x6Fu  /* 9 */
};

static const uint32_t segment_pins[8] = {
    (1u << 12), /* segA PF12 */
    (1u << 3),  /* segB PB3  */
    (1u << 6),  /* segC PD6  */
    (1u << 4),  /* segD PD4  */
    (1u << 14), /* segE PE14 */
    (1u << 15), /* segF PB15 */
    (1u << 5),  /* segG PD5  */
    (1u << 3)   /* decimal point PD3 */
};

static GPIO_TypeDef *segment_ports[8] = {
    GPIOF, /* segA */
    GPIOB, /* segB */
    GPIOD, /* segC */
    GPIOD, /* segD */
    GPIOE, /* segE */
    GPIOB, /* segF */
    GPIOD, /* segG */
    GPIOD  /* decimal point */
};

static const uint32_t digit_pins[4] = {
    (1u << 13), /* digit1 PF13 */
    (1u << 14), /* digit2 PB14 */
    (1u << 13), /* digit3 PB13 */
    (1u << 10)  /* digit4 PA10 */
};

static GPIO_TypeDef *digit_ports[4] = {
    GPIOF,
    GPIOB,
    GPIOB,
    GPIOA
};

static void display_all_off(void)
{
    /* Turn off all segments (set low) */
    GPIOF->BSRR = (1u << 12) << 16U;
    GPIOB->BSRR = ((1u << 3) << 16U) | ((1u << 15) << 16U);
    GPIOE->BSRR = (1u << 14) << 16U;
    GPIOD->BSRR = ((1u << 6) << 16U) | ((1u << 4) << 16U) | ((1u << 5) << 16U) | ((1u << 3) << 16U);

    /* Turn off all digits (set high for common cathode) */
    GPIOF->BSRR = (1u << 13);
    GPIOB->BSRR |= (1u << 14) | (1u << 13);
    GPIOA->BSRR = (1u << 10);
}

static void display_digit(uint8_t digit_index, uint8_t value, bool enable_dp)
{
    display_all_off();

    if (value != DIGIT_BLANK) {
        uint8_t pattern = segment_map[value];
        for (uint32_t bit = 0; bit < 7u; ++bit) {
            const uint32_t pin_mask = segment_pins[bit];
            if ((pattern >> bit) & 1u) {
                ((GPIO_TypeDef *)segment_ports[bit])->BSRR = pin_mask;
            }
        }
    }

    if (enable_dp) {
        GPIOD->BSRR = segment_pins[7];
    }

    digit_ports[digit_index]->BSRR = digit_pins[digit_index] << 16U;
}

static uint32_t pow10_32(uint8_t exponent) {
    uint32_t result = 1u;
    while (exponent--) {
        result *= 10u;
    }
    return result;
}

static uint8_t digit_count(uint32_t value) {
    if (value == 0u) {
        return 1u;
    }

    uint8_t count = 0u;
    while (value != 0u) {
        value /= 10u;
        count++;
    }
    return count;
}

static void format_integer_value(uint16_t number, uint8_t digits[4]) {
    for (uint8_t i = 0u; i < 4u; ++i) {
        digits[i] = DIGIT_BLANK;
    }

    if (number == 0u) {
        digits[3] = 0u;
        return;
    }

    int8_t position = 3;
    while (number != 0u && position >= 0) {
        digits[position--] = (uint8_t)(number % 10u);
        number /= 10u;
    }
}

static void format_float_value(float value, uint8_t digits[4], int8_t *dp_index) {
    if (value < 0.0f) {
        value = 0.0f;
    }

    if (value >= 10000.0f) {
        value = 9999.0f;
    }

    uint32_t integer_part = (uint32_t)value;
    float fractional = value - (float)integer_part;
    if (fractional < 0.0f) {
        fractional = 0.0f;
    }

    if (fractional < 0.00001f || integer_part >= 1000u) {
        *dp_index = -1;
        format_integer_value((uint16_t)integer_part, digits);
        return;
    }

    uint32_t max_decimals;
    if (integer_part < 10u) {
        max_decimals = 3u;
    } else if (integer_part < 100u) {
        max_decimals = 2u;
    } else {
        max_decimals = 1u;
    }

    uint32_t scale = pow10_32(max_decimals);
    uint32_t scaled_value = (uint32_t)(value * (float)scale + 0.5f);
    uint32_t max_scaled = 9999u * scale;
    if (scaled_value > max_scaled) {
        scaled_value = max_scaled;
    }

    uint32_t decimals = max_decimals;
    while (decimals > 1u && (scaled_value % 10u) == 0u) {
        scaled_value /= 10u;
        decimals--;
    }

    uint32_t divider = pow10_32(decimals);
    uint32_t display_integer = scaled_value / divider;
    uint32_t display_fraction = scaled_value % divider;

    if (display_integer >= 1000u) {
        *dp_index = -1;
        format_integer_value((uint16_t)display_integer, digits);
        return;
    }

    uint8_t integer_digits = digit_count(display_integer);
    if (integer_digits == 0u) {
        integer_digits = 1u;
    }

    for (uint8_t i = 0u; i < 4u; ++i) {
        digits[i] = DIGIT_BLANK;
    }

    int8_t position = 3;
    for (uint32_t i = 0u; i < decimals; ++i) {
        digits[position--] = (uint8_t)(display_fraction % 10u);
        display_fraction /= 10u;
    }

    for (uint32_t i = 0u; i < integer_digits; ++i) {
        digits[position--] = (uint8_t)(display_integer % 10u);
        display_integer /= 10u;
    }

    *dp_index = (int8_t)(3 - decimals - (integer_digits - 1u));
}

static void short_delay(void) {
    for (volatile uint32_t i = 0u; i < 1000u; ++i) {
        __asm("nop");
    }
}

void segment_display_init(void) {
    display_all_off();
}

void segment_display_clear(void) {
    display_all_off();
}

void segment_display_show_number(uint16_t number) {
    uint8_t digits[4];
    format_integer_value(number, digits);

    const uint32_t cycles = 125u;
    for (uint32_t cycle = 0u; cycle < cycles; ++cycle) {
        for (uint8_t idx = 0u; idx < 4u; ++idx) {
            display_digit(idx, digits[idx], false);
            short_delay();
        }
    }

    display_all_off();
}

void segment_display_show_float(float value) {
    uint8_t digits[4];
    int8_t dp_index;
    format_float_value(value, digits, &dp_index);

    const uint32_t cycles = 125u;
    for (uint32_t cycle = 0u; cycle < cycles; ++cycle) {
        for (uint8_t idx = 0u; idx < 4u; ++idx) {
            display_digit(idx, digits[idx], dp_index == (int8_t)idx);
            short_delay();
        }
    }

    display_all_off();
}

void segment_display_show_digit(uint8_t digit, uint8_t position, uint8_t dp) {
    display_digit(position, digit, dp != 0);
}

void segment_display_show_temperature(float temperature) {
    // Round to nearest 0.1°C
    int16_t temp_int = (int16_t)(temperature * 10.0f);
    
    // Clamp to valid range (-99.9 to 99.9)
    if (temp_int < -999) temp_int = -999;
    if (temp_int > 999) temp_int = 999;
    
    // Format as [tens][ones].[tenths][C_symbol]
    uint8_t digits[4];
    
    // Extract sign
    uint8_t is_negative = 0;
    if (temp_int < 0) {
        is_negative = 1;
        temp_int = -temp_int;
    }
    
    // Extract decimal digit values
    uint8_t tenths = temp_int % 10;
    uint8_t ones = (temp_int / 10) % 10;
    uint8_t tens = (temp_int / 100) % 10;
    
    // Build digit array
    // If negative and tens=0, show minus on first digit
    if (is_negative && tens == 0) {
        digits[0] = DIGIT_BLANK;  // Will display as blank (no minus symbol available)
    } else {
        digits[0] = tens;
    }
    digits[1] = ones;
    digits[2] = tenths;
    // digits[3] = 'C' indicator (0x39 = segments for C: A, F, E, D)
    
    // Display with cycling
    const uint32_t cycles = 125u;
    const uint8_t C_SYMBOL = 0x39;  // 7-segment pattern for 'C'
    
    for (uint32_t cycle = 0u; cycle < cycles; ++cycle) {
        for (uint8_t idx = 0u; idx < 3u; ++idx) {
            bool show_dp = (idx == 1);  // Decimal point after ones digit
            display_digit(idx, digits[idx], show_dp);
            short_delay();
        }
        // Display 'C' symbol on digit 3
        display_all_off();
        // Manually set segments for 'C' on digit 3
        uint32_t pattern = C_SYMBOL;
        for (uint32_t bit = 0; bit < 7u; ++bit) {
            const uint32_t pin_mask = segment_pins[bit];
            if ((pattern >> bit) & 1u) {
                ((GPIO_TypeDef *)segment_ports[bit])->BSRR = pin_mask;
            }
        }
        digit_ports[3]->BSRR = digit_pins[3] << 16U;
        short_delay();
    }
    
    display_all_off();
}
