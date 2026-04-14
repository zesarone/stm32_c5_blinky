#ifndef SEGMENT_DISPLAY_H
#define SEGMENT_DISPLAY_H

#include <stdint.h>
#include "stm32c5a3xx.h"

// Pin definitions for KYX5461AS 7-segment display
#define SEG_A_PORT GPIOF
#define SEG_A_PIN  12
#define SEG_B_PORT GPIOB
#define SEG_B_PIN  3
#define SEG_C_PORT GPIOD
#define SEG_C_PIN  6
#define SEG_D_PORT GPIOD
#define SEG_D_PIN  4
#define SEG_E_PORT GPIOE
#define SEG_E_PIN  14
#define SEG_F_PORT GPIOB
#define SEG_F_PIN  15
#define SEG_G_PORT GPIOD
#define SEG_G_PIN  5
#define SEG_DP_PORT GPIOD
#define SEG_DP_PIN  3

#define DIGIT1_PORT GPIOF
#define DIGIT1_PIN  13
#define DIGIT2_PORT GPIOB
#define DIGIT2_PIN  14
#define DIGIT3_PORT GPIOB
#define DIGIT3_PIN  13
#define DIGIT4_PORT GPIOA
#define DIGIT4_PIN  10

#define DIGIT_COUNT 4

// Segment patterns for digits 0-9
#define SEG_0 0x3F  // 0b00111111
#define SEG_1 0x06  // 0b00000110
#define SEG_2 0x5B  // 0b01011011
#define SEG_3 0x4F  // 0b01001111
#define SEG_4 0x66  // 0b01100110
#define SEG_5 0x6D  // 0b01101101
#define SEG_6 0x7D  // 0b01111101
#define SEG_7 0x07  // 0b00000111
#define SEG_8 0x7F  // 0b01111111
#define SEG_9 0x6F  // 0b01101111

/**
 * Initialize the 7-segment display hardware.
 *
 * GPIO pin direction is configured in system_init(). This function initializes
 * the display driver to a known-off state.
 */
void segment_display_init(void);

/**
 * Clear the 7-segment display (turn off all segments and digits).
 */
void segment_display_clear(void);

/**
 * Display a number on the 7-segment display using multiplexing.
 *
 * @param number Number to display (0-9999).
 *
 * This function uses time-multiplexing to show all 4 digits.
 * Call this function repeatedly in a loop for continuous display.
 */
void segment_display_show_number(uint16_t number);

/**
 * Display a floating-point number on the 7-segment display.
 *
 * @param value Float value to display.
 *
 * Values are clamped to the supported unsigned 4-digit display range.
 * Leading zeros are suppressed and trailing fractional zeros are trimmed.
 */
void segment_display_show_float(float value);

/**
 * Display a single digit at a specific position.
 *
 * @param digit    Digit to display (0-9).
 * @param position Position (0=leftmost, 3=rightmost).
 * @param dp       Nonzero to show decimal point.
 */
void segment_display_show_digit(uint8_t digit, uint8_t position, uint8_t dp);

/**
 * Display temperature value with °C unit indicator.
 * Shows temperature on digits 0-2 and a trailing C symbol on digit 3.
 *
 * @param temperature Temperature in Celsius to display.
 */
void segment_display_show_temperature(float temperature);

#endif // SEGMENT_DISPLAY_H