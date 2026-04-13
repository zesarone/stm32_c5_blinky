#include "led.h"

static const LedConfig leds[LED_COUNT] = {
    { LD1_PORT, LD1_PIN, 0 },
    { LD2_PORT, LD2_PIN, 1 },
    { LD3_PORT, LD3_PIN, 1 },
};

static void set_pin_output(GPIO_TypeDef *port, uint32_t pin) {
    uint32_t shift = pin * 2;
    port->MODER &= ~(3U << shift);
    port->MODER |=  (1U << shift);
}

static inline void led_write(const LedConfig *led, uint8_t on) {
    uint32_t mask = PIN_MASK(led->pin);

    if (on) {
        if (led->activeLow) {
            led->port->ODR &= ~mask;
        } else {
            led->port->ODR |= mask;
        }
    } else {
        if (led->activeLow) {
            led->port->ODR |= mask;
        } else {
            led->port->ODR &= ~mask;
        }
    }
}

static inline void led_on(const LedConfig *led) {
    led_write(led, 1);
}

static inline void led_off(const LedConfig *led) {
    led_write(led, 0);
}

void led_init(void) {
    for (uint32_t i = 0; i < LED_COUNT; i++) {
        set_pin_output(leds[i].port, leds[i].pin);
        led_off(&leds[i]);
    }
}

void led_all_off(void) {
    for (uint32_t i = 0; i < LED_COUNT; i++) {
        led_off(&leds[i]);
    }
}

void led_set(uint8_t index, uint8_t on) {
    if (index >= LED_COUNT) {
        return;
    }
    if (on) {
        led_on(&leds[index]);
    } else {
        led_off(&leds[index]);
    }
}

