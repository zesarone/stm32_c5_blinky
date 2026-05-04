# STM32 C5 Home Experiment Project Requirements

Target board manual:
https://www.st.com/resource/en/user_manual/um3616-stm32c5-nucleo144-board-mb2310-stmicroelectronics.pdf

or locally: 
./um3616-stm32c5-nucleo144-board-mb2310-stmicroelectronics.pdf

This document describes the target behavior for the current firmware revision.
Most sections remain aligned with the existing codebase, while the joystick-related sections define the next required behavior before the code is updated.

## Board manual cross-reference

- The board-specific statements in this document have been cross-checked against UM3616 for the NUCLEO-C5A3ZG.
- The verified board-level items are:
	- USER button `B1` on `PC13`
	- user LEDs `LD1`, `LD2`, and `LD3`
	- button active level and debounce expectations
	- default LED routing on the stock board configuration
- The external 4-digit seven-segment display is project-specific hardware and is not part of the NUCLEO-C5A3ZG board manual.
- The external two-axis joystick used by State 2 is project-specific hardware and is not part of the NUCLEO-C5A3ZG board manual.
- The external rotary encoder used by State 3 is project-specific hardware and is not part of the NUCLEO-C5A3ZG board manual.

## Project structure

- Board and peripheral initialization is implemented in `init.c`.
- LED control is implemented in `led.c`.
- Seven-segment display rendering is implemented in `segment_display.c`.
- Each application state is implemented in its own source file:
	- `state_1.c`
	- `state_2.c`
	- `state_3.c`
- The top-level state machine is implemented in `main.c`.

## Top-level firmware behavior

### Startup and initialization

- On reset, the firmware shall execute `system_init()` before entering the main loop.
- `system_init()` shall:
	- enable HSI
	- enable GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF, GPIOG, and GPIOH clocks
	- enable the ADC12 clock
	- configure every signal listed in the Peripheral Configuration section
	- initialize the LED and display modules
- After `system_init()` and before entering the main loop, the firmware shall perform a one-time joystick calibration.
- After initialization completes, the firmware shall enter State 1.
- After initialization completes, the display shall be controlled entirely by the active state logic.

### State machine

- The firmware contains exactly three runtime states.
- The active state shall advance on a debounced rising edge of the USER button.
- The transition order is `State 1 -> State 2 -> State 3 -> State 1`.
- A simple software debounce delay is applied after a detected USER button press.
- The active state LED shall update immediately after the state change.
- The firmware shall not show a 2-second state-number splash screen during transitions.
- The firmware shall not add any other transition animation or state-entry display phase.

## Peripheral configuration

### Peripheral summary

| Peripheral | Signal | Pin | Required configuration | Notes |
| --- | --- | --- | --- | --- |
| USER button | `B1` | `PC13` | digital input | active-high, debounced in software, must never be driven as an output |
| State LED | `LD1` | `PA5` | digital output | active-high, indicates State 1, default `SB8` routing |
| State LED | `LD2` | `PG1` | digital output | active-low, indicates State 2 |
| State LED | `LD3` | `PG2` | digital output | active-low, indicates State 3 |
| 7-segment display | `segA` | `PF12` | digital output | segment line |
| 7-segment display | `segB` | `PB3` | digital output | segment line |
| 7-segment display | `segC` | `PD6` | digital output | segment line |
| 7-segment display | `segD` | `PD4` | digital output | segment line |
| 7-segment display | `segE` | `PE14` | digital output | segment line |
| 7-segment display | `segF` | `PB15` | digital output | segment line |
| 7-segment display | `segG` | `PD5` | digital output | segment line |
| 7-segment display | `decimal point` | `PD3` | digital output | decimal-point line |
| 7-segment display | `digit1` | `PF13` | digital output | leftmost digit select |
| 7-segment display | `digit2` | `PB14` | digital output | second digit select |
| 7-segment display | `digit3` | `PB13` | digital output | third digit select |
| 7-segment display | `digit4` | `PA10` | digital output | rightmost digit select |
| Joystick | `VRX` | `PA4` | analog input | X-axis input |
| Joystick | `VRY` | `PB0` | analog input | Y-axis input |
| Joystick | `SW` | `PC0` | digital input with pull-up | active-low push-switch, controls decimal-point overlay in State 2 |
| Rotary encoder | `SW` | `PA2` | digital input with pull-up | currently unused by the application |
| Rotary encoder | `DT` | `PA3` | digital input with pull-up | quadrature phase input |
| Rotary encoder | `CLK` | `PC3` | digital input with pull-up | quadrature phase input |

### USER button

- The USER button is the on-board `B1` button on `PC13`.
- The electrical behavior is active-high: pressed reads high and released reads low.
- The board does not provide a hardware debounce filter for `B1`, so debounce shall be handled in software.
- `PC13` must remain configured as an input and must never be driven low as an output.
- The USER button is used only for state changes.

### State LEDs

- State 1 uses `LD1`.
- State 2 uses `LD2`.
- State 3 uses `LD3`.
- Exactly one state LED shall be active at a time.
- The firmware shall turn all state LEDs off before enabling the LED for the newly selected state.
- The board-level LED routing assumptions are:
	- `LD1` is connected to `PA5` in the default board configuration and is active-high
	- `LD2` is connected to `PG1` and is used as active-low in the firmware
	- `LD3` is connected to `PG2` and is used as active-low in the firmware

### Seven-segment display

- The display is an external 4-digit KYX5461AS-style seven-segment display with decimal point.
- The display is driven by multiplexing.
- Digit numbering is left-to-right: digit 1 is the leftmost digit and digit 4 is the rightmost digit.

Segment layout:

```text
Digit 1   Digit 2   Digit 3   Digit 4
|--A--|   |--A--|   |--A--|   |--A--|
F     B   F     B   F     B   F     B
|--G--|   |--G--|   |--G--|   |--G--|
E     C   E     C   E     C   E     C
|--D--| * |--D--| * |--D--| * |--D--| *
```

#### Numeric display support

- `segment_display_show_number()` displays unsigned integer values in the range `0..9999`.
- Leading zeros are suppressed.
- Zero is rendered as a single `0` on the rightmost digit.
- `segment_display_show_float()` formats non-negative floating-point values to fit within four digits.
- The float formatter keeps as many decimal places as will fit:
	- up to 3 decimals for values below `10`
	- up to 2 decimals for values below `100`
	- up to 1 decimal for values below `1000`
- Trailing fractional zeros are trimmed when possible.
- Float values below `0` are clamped to `0`.
- Float values at or above `10000` are capped to `9999` for display.

### Joystick

- State 2 uses an external two-axis analog joystick with push switch.
- `PA4` carries the X-axis analog signal.
- `PB0` carries the Y-axis analog signal.
- `PC0` carries the joystick push-switch signal.
- At startup, the joystick may be assumed to be released and spring-centered.
- The firmware shall sample `VRX` and `VRY` during startup and store those readings as the centered reference for State 2.
- The firmware shall continuously sample both analog joystick axes while State 2 is active.
- A center dead zone shall be applied around the startup-calibrated center so that small analog variations around the resting position do not cause the indicated direction to flicker.
- The Y axis shall be classified into three display rows: up, vertical-center, and down.
- The X axis shall use the full horizontal resolution of the display grid.
- Because the display has an even number of horizontal columns, the centered X position shall be rendered with the two middle columns lit together.
- The firmware shall interpret axis polarity so that the display matches physical motion:
	- pushing the joystick left lights the left indicator
	- pushing the joystick right lights the right indicator
	- pushing the joystick up lights the up indicator
	- pushing the joystick down lights the down indicator
- For the current joystick wiring, `VRX` shall be interpreted so that the high region means left and the low region means right.
- The joystick switch input on `PC0` shall be configured with an internal pull-up so the released state is deterministic.
- The joystick switch shall be treated as active-low: released reads high and pressed reads low.
- The joystick switch shall be used only as a display overlay control in State 2.
- When the joystick switch is pressed, all four decimal points shall be lit.
- When the joystick switch is released, all four decimal points shall turn off immediately.
- The joystick switch overlay shall not latch, blink, or remain on after release.

### Rotary encoder

- State 3 uses an external rotary encoder.
- `DT` and `CLK` are decoded in software to determine direction of rotation.
- Rotating the encoder in one direction increments the State 3 counter by one step.
- Rotating the encoder in the opposite direction decrements the State 3 counter by one step.
- The encoder push switch on `PA2` is currently not used by the application.

## Runtime state behavior

### State 1: Prime sieve display

- State 1 uses a sieve-based prime generator over the range `0..9999`.
- The prime sequence starts from `2` after firmware reset.
- Each State 1 update displays the next prime in the sequence.
- When the sieve reaches the end of the supported range, it is reinitialized and the sequence starts again from `2`.
- Values above `9999` are not displayed.

### State 2: Joystick position indicator

- State 2 uses the seven-segment display as a direction indicator and does not show a numeric value.
- The four digits form a fixed spatial indicator in a 3x8 grid 3 vertical and 8 horizontal:


```text

		   	   D1   	D2   	   D3        D4
row	0		|--A--|   |--A--|   |--A--|   |--A--|
			F     B   F     B   F     B   F     B
row	1		|--G--|   |--G--|   |--G--|   |--G--|
			E     C   E     C   E     C   E     C
row	2		|--D--| * |--D--| * |--D--| * |--D--| *

columns:    0     1   2     3   4     5   6     7
```

light up only the row and column for each position of the joystick

- The horizontal grid columns are rendered as:
	- column 0, left-most: segments `E` and `F` on digit 1
	- column 1, left-inner: segments `B` and `C` on digit 1
	- column 2, horizontal-center-left: segments `E` and `F` on digit 2
	- columns 3 and 4, horizontal-center: segments `B` and `C` on digit 2 and segments `E` and `F` on digit 3
	- column 5, horizontal-center-right: segments `B` and `C` on digit 3
	- column 6, right-inner: segments `E` and `F` on digit 4
	- column 7, right-most: segments `B` and `C` on digit 4
- The vertical grid rows are rendered on the digit or digits selected by the horizontal position:
	- up uses segment `A`
	- vertical-center uses segment `G`
	- down uses segment `D`
- For horizontal column rendering, the lit side segment depends on the active row:
	- in row 0, left-side columns use only `F` and right-side columns use only `B`
	- in row 1, left-side columns use `E` and `F` and right-side columns use `B` and `C`
	- in row 2, left-side columns use only `E` and right-side columns use only `C`

- If only the X axis is displaced, the matching horizontal position indicator and the vertical-center indicator shall be lit on that same horizontal position.
- If only the Y axis is displaced, the horizontal-center indicator and the matching up or down indicator shall be lit.
- If both axes are displaced, the matching horizontal position indicator and the matching up or down indicator shall be lit simultaneously to indicate the joystick position.
- If both axes are within the dead zone, the horizontal-center indicator and the vertical-center indicator shall be lit so that the centered joystick position is still shown.
- Digits that are not part of the current direction shall remain blank.
- The decimal-point overlay is independent of the direction segments and is controlled only by the joystick switch state.

### State 3: Counter

- State 3 displays an unsigned counter value in the range `0..9999`.
- The counter starts at `0` after firmware reset.
- Rotating the encoder in one direction increments the counter by one step.
- Rotating the encoder in the opposite direction decrements the counter by one step.
- The counter wraps in the range `0..9999`.
- The counter value is retained until the firmware is reset.
- The display continuously shows the current counter value with no automatic ticking.

## Implementation notes and non-goals

- The firmware is bare-metal and does not use HAL display or state-management helpers.
- There is no UART output or logging requirement.
- There is no dedicated state-entry animation or state-number presentation phase.
- This document defines the target behavior for the next firmware revision, including the joystick-driven State 2 display, even where the current code still reflects the earlier temperature-display behavior.
