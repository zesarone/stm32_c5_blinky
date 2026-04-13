#ifndef LED_H
#define LED_H

#include <stdint.h>
#include "stm32c5a3xx.h"

/**
 * LED pin definitions for the NUCLEO-C5A3ZG board.
 * LD1 is on PA5, LD2 is on PG1, LD3 is on PG2.
 */
#define LD1_PORT GPIOA
#define LD1_PIN  5  // PA5
#define LD2_PORT GPIOG
#define LD2_PIN  1  // PG1
#define LD3_PORT GPIOG
#define LD3_PIN  2  // PG2

/**
 * Number of LEDs managed by the LED library.
 */
#define LED_COUNT 3

/**
 * Create a mask for the selected GPIO pin number.
 * Example: PIN_MASK(5) -> 0x20
 */
#define PIN_MASK(pin) (1U << (pin))

/**
 * LED configuration stored by the LED library.
 * @param port Pointer to GPIO port register block.
 * @param pin Pin number within that port.
 * @param activeLow True when LED is on with a low GPIO level.
 */
typedef struct {
    GPIO_TypeDef *port;
    uint32_t pin;
    uint8_t activeLow;
} LedConfig;

/**
 * Initialize all LEDs managed by the library.
 *
 * This function enables each LED pin as a GPIO output and sets the
 * LED state to off using the board-specific active-high/active-low
 * configuration.
 */
void led_init(void);

/**
 * Turn off every LED managed by the library.
 *
 * This is useful before selecting a new state so that only the
 * requested LED remains lit.
 */
void led_all_off(void);

/**
 * Set the state of a single LED.
 *
 * @param index LED index from 0 to LED_COUNT - 1.
 *              0 = LD1, 1 = LD2, 2 = LD3.
 * @param on    Nonzero value turns the LED on, zero turns it off.
 *
 * This function handles active-low LEDs internally, so callers do not
 * need to worry about the pin polarity.
 */
void led_set(uint8_t index, uint8_t on);

#endif // LED_H
