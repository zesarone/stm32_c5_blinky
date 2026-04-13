#ifndef INIT_H
#define INIT_H

#include <stdint.h>

/**
 * USER button pin on the board.
 * This pin is configured as an input by system_init().
 */
#define USER_BUTTON_PIN 13  // PC13

/**
 * Initialize board peripherals used by the application.
 *
 * This function enables GPIO clocks, configures the USER button pin as
 * input, and calls led_init() to prepare LED output pins.
 */
void system_init(void);

#endif // INIT_H
