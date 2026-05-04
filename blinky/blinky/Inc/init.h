#ifndef INIT_H
#define INIT_H

#include <stdint.h>

/**
 * USER button pin on the board.
 * This pin is configured as an input by system_init().
 */
#define USER_BUTTON_PIN 13  // PC13

/**
 * Rotary encoder pins.
 * SW is currently unused by the application, but it is configured as an input.
 */
#define ROTARY_ENCODER_SW_PIN   2U   // PA2
#define ROTARY_ENCODER_DT_PIN   3U   // PA3
#define ROTARY_ENCODER_CLK_PIN  3U   // PC3

/**
 * Initialize board peripherals used by the application.
 *
 * This function enables GPIO clocks, configures the USER button and rotary
 * encoder input pins, and calls led_init() to prepare LED output pins.
 */
void system_init(void);

#endif // INIT_H
