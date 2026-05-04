#ifndef STATE_2_H
#define STATE_2_H

/**
 * State 2: Joystick position indicator
 *
 * Samples the joystick at startup to capture the spring-centered rest position
 * and later renders direction relative to that calibrated center.
 */
void state_2_init(void);

/**
 * State 2: Joystick position indicator
 *
 * Reads the external joystick axes and renders the current direction on the
 * four-digit seven-segment display. The joystick switch overlays the decimal
 * points while pressed.
 */
void state_2_run(void);

#endif // STATE_2_H