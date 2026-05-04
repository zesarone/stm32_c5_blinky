#ifndef INIT_H
#define INIT_H

#include <stdint.h>
#include "stm32c5xx.h"

/**
 * USER button pin on the board.
 * This pin is configured as an input by system_init().
 */
#define USER_BUTTON_PIN 13  // PC13

/**
 * Joystick pins.
 * VRX and VRY are analog inputs. SW is configured as a pulled digital input.
 */
#define JOYSTICK_VRX_PIN  4U   // PA4
#define JOYSTICK_VRY_PIN  0U   // PB0
#define JOYSTICK_SW_PORT  GPIOC
#define JOYSTICK_SW_PIN   0U   // PC0

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
 * encoder input pins, configures the reserved RMII Ethernet pins, and calls
 * led_init() to prepare LED output pins.
 */
void system_init(void);

#endif // INIT_H
