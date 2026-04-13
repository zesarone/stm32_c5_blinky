#include <stdbool.h>
#include "stm32c5xx.h"
#include "init.h"
#include "segment_display.h"

/* Segment mapping for digits 0-9 using active-high segments A..G. */
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
    GPIOF->BSRR = ((1u << 12) << 16U) | (1u << 13);
    GPIOB->BSRR = ((1u << 3) << 16U) | ((1u << 15) << 16U) | (1u << 14) | (1u << 13);
    GPIOE->BSRR = ((1u << 14) << 16U);
    GPIOD->BSRR = ((1u << 6) << 16U) | ((1u << 4) << 16U) | ((1u << 5) << 16U) | ((1u << 3) << 16U);
    GPIOA->BSRR = (1u << 10);
}

static void display_digit(uint8_t digit_index, uint8_t value, bool enable_dp)
{
    display_all_off();

    uint8_t pattern = segment_map[value];
    for (uint32_t bit = 0; bit < 7u; ++bit) {
        const uint32_t pin_mask = segment_pins[bit];
        if ((pattern >> bit) & 1u) {
            ((GPIO_TypeDef *)segment_ports[bit])->BSRR = pin_mask;
        }
    }

    if (enable_dp) {
        GPIOD->BSRR = segment_pins[7];
    }

    digit_ports[digit_index]->BSRR = digit_pins[digit_index] << 16U;
}

static void format_value(float value, uint8_t digits[4], int8_t *dp_index)
{
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

    if (integer_part >= 1000u || fractional < 0.00001f) {
        *dp_index = -1;
        uint32_t scaled = integer_part;
        for (int i = 3; i >= 0; --i) {
            digits[i] = (uint8_t)(scaled % 10u);
            scaled /= 10u;
        }
        return;
    }

    uint32_t scale;
    if (integer_part < 10u) {
        scale = 1000u;
        *dp_index = 0;
    } else if (integer_part < 100u) {
        scale = 100u;
        *dp_index = 1;
    } else {
        scale = 10u;
        *dp_index = 2;
    }

    uint32_t scaled = integer_part * scale + (uint32_t)(fractional * (float)scale);
    scaled %= 10000u;
    for (int i = 3; i >= 0; --i) {
        digits[i] = (uint8_t)(scaled % 10u);
        scaled /= 10u;
    }
}

void segment_display_init(void)
{
    display_all_off();
}

void segment_display_show(float value)
{
    uint8_t digits[4];
    int8_t dp_position;
    format_value(value, digits, &dp_position);

    const uint32_t cycles = 125u;
    for (uint32_t cycle = 0u; cycle < cycles; ++cycle) {
        for (uint8_t idx = 0; idx < 4u; ++idx) {
            display_digit(idx, digits[idx], dp_position == (int8_t)idx);
            board_delay_ms(1u);
        }
    }

    display_all_off();
}
