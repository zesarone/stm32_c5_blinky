#include <stdint.h>
#include "init.h"
#include "segment_display.h"
#include "stm32c5a3xx.h"

static volatile uint16_t counter = 0u;
static volatile uint8_t previous_encoder_state = 3u;
static volatile int8_t encoder_accumulator = 0;

static const int8_t transition_table[16] = {
    0, -1,  1,  0,
    1,  0,  0, -1,
    -1,  0,  0,  1,
    0,  1, -1,  0
};

static uint32_t pin_mask(uint32_t pin)
{
    return 1u << pin;
}

static uint8_t read_encoder_clk_raw(void)
{
    return (GPIOC->IDR & pin_mask(ROTARY_ENCODER_CLK_PIN)) != 0u;
}

static uint8_t read_encoder_dt_raw(void)
{
    return (GPIOA->IDR & pin_mask(ROTARY_ENCODER_DT_PIN)) != 0u;
}

static uint8_t read_encoder_state(void)
{
    uint8_t clk = read_encoder_clk_raw();
    uint8_t dt = read_encoder_dt_raw();

    return (uint8_t)((clk << 1u) | dt);
}

static void increment_counter(void)
{
    if (counter >= 9999u) {
        counter = 0u;
    } else {
        counter++;
    }
}

static void decrement_counter(void)
{
    if (counter == 0u) {
        counter = 9999u;
    } else {
        counter--;
    }
}

void SysTick_Handler(void)
{
    uint8_t current_state = read_encoder_state();
    uint8_t transition_index;
    int8_t delta;

    if (current_state == previous_encoder_state) {
        return;
    }

    transition_index = (uint8_t)((previous_encoder_state << 2u) | current_state);
    delta = transition_table[transition_index];
    previous_encoder_state = current_state;

    if (delta != 0) {
        encoder_accumulator = (int8_t)(encoder_accumulator + delta);

        if (encoder_accumulator >= 2) {
            increment_counter();
            encoder_accumulator = 0;
        } else if (encoder_accumulator <= -2) {
            decrement_counter();
            encoder_accumulator = 0;
        }
    }
}

void state_3_run(void)
{
    uint16_t counter_snapshot = counter;
    segment_display_scan_number_step(counter_snapshot);
}