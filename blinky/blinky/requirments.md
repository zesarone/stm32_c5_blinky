# STM32 C5 Home Experiment Project Requirements

Target board manual:
https://www.st.com/resource/en/user_manual/um3616-stm32c5-nucleo144-board-mb2310-stmicroelectronics.pdf

This document describes the behavior implemented in the current codebase. It is a refined, implementation-aligned specification rather than the earlier draft requirement list.

## Board manual cross-reference

- The board-specific statements in this document have been cross-checked against UM3616 for the NUCLEO-C5A3ZG.
- The verified board-level items are:
	- USER button `B1` on `PC13`
	- user LEDs `LD1`, `LD2`, and `LD3`
	- button active level and debounce expectations
	- default LED routing on the stock board configuration
- The external 4-digit seven-segment display is project-specific hardware and is not part of the NUCLEO-C5A3ZG board manual.
- The external rotary encoder used by State 3 is project-specific hardware and is not part of the NUCLEO-C5A3ZG board manual.
- The internal temperature-sensor conversion constants come from MCU documentation, not from the Nucleo board manual.

## Project structure

- Board and peripheral initialization is implemented in `init.c`.
- LED control is implemented in `led.c`.
- Seven-segment display rendering is implemented in `segment_display.c`.
- Each application state is implemented in its own source file:
	- `state_1.c`
	- `state_2.c`
	- `state_3.c`
- The top-level state machine is implemented in `main.c`.

## Board initialization

- `system_init()` is responsible for bringing the board to a usable runtime state before the main loop starts.
- The current implementation:
	- enables HSI
	- enables GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF, and GPIOG clocks
	- enables the ADC12 clock
	- configures the USER button pin as input
	- configures the rotary encoder pins as inputs with pull-ups
	- configures the seven-segment display GPIO pins as outputs
	- initializes the LED and display modules
- Manual-backed board detail:
	- the USER button is `B1` on `PC13`
	- button logic is active-high when pressed and low when released
	- the board does not provide a hardware debounce filter for `B1`, so debounce must be handled in software
	- `PC13` must remain configured as an input and must not be driven low as an output

## State machine behavior

- The firmware contains exactly three runtime states.
- The active state advances on a USER button rising edge.
- The transition order is `State 1 -> State 2 -> State 3 -> State 1`.
- A simple software debounce delay is applied after a detected button press.
- The active state LED is updated immediately after the state change.
- The currently implemented firmware does not show a 2-second state-number splash screen during transitions.

## LED state indication

- State 1 uses LD1.
- State 2 uses LD2.
- State 3 uses LD3.
- LED control is abstracted behind helper functions in `led.c`.
- The main loop turns all LEDs off before enabling the LED for the newly selected state.
- Manual-backed board detail:
	- `LD1` is connected to `PA5` in the default board configuration and is active-high
	- `LD2` is connected to `PG1`
	- `LD3` is connected to `PG2`
	- the current firmware assumes the default solder-bridge configuration for `LD1` (`SB8` path), not the alternate `PG0` routing
	- the `led.c` polarity settings match the implemented board usage: `LD1` active-high, `LD2` active-low, `LD3` active-low

## Seven-segment display

- A 4-digit KYX5461AS-style seven-segment display with decimal point is driven by multiplexing.
- This display is external application hardware and is not described by the NUCLEO-C5A3ZG board manual.
- The implemented GPIO mapping is:
	- `segA` -> `PF12`
	- `segB` -> `PB3`
	- `segC` -> `PD6`
	- `segD` -> `PD4`
	- `segE` -> `PE14`
	- `segF` -> `PB15`
	- `segG` -> `PD5`
	- `decimal point` -> `PD3`
	- `digit1` -> `PF13`
	- `digit2` -> `PB14`
	- `digit3` -> `PB13`
	- `digit4` -> `PA10`

Segment layout:

```text
Digit 1   Digit 2   Digit 3   Digit 4
|--A--|   |--A--|   |--A--|   |--A--|
F     B   F     B   F     B   F     B
|--G--|   |--G--|   |--G--|   |--G--|
E     C   E     C   E     C   E     C
|--D--| * |--D--| * |--D--| * |--D--| *
```

### Integer display behavior

- `segment_display_show_number()` displays unsigned integer values in the range `0..9999`.
- Leading zeros are suppressed.
- Zero is rendered as a single `0` on the rightmost digit.

### Generic float display behavior

- `segment_display_show_float()` formats non-negative floating-point values to fit within four digits.
- The formatter keeps as many decimal places as will fit:
	- up to 3 decimals for values below `10`
	- up to 2 decimals for values below `100`
	- up to 1 decimal for values below `1000`
- Trailing fractional zeros are trimmed when possible.
- Values below `0` are clamped to `0`.
- Values at or above `10000` are capped to `9999` for display.

### Temperature display behavior

- State 2 uses the dedicated `segment_display_show_temperature()` path instead of the generic float formatter.
- Temperature is displayed with one decimal place and a trailing `C` symbol on the fourth digit.
- The display format is effectively `TT.TC`.
- The temperature renderer supports values within `-99.9..99.9`, but the current seven-segment implementation does not render a minus sign.
- Negative temperatures therefore do not display a visible sign.

## State 1: Prime sieve display

- State 1 uses a sieve-based prime generator over the range `0..9999`.
- The sequence starts from prime `2`.
- Each call to `state_1_run()` displays one prime value.
- When the sieve reaches the end of the supported range, it is reinitialized and the sequence starts again from `2`.
- Values above `9999` are not displayed.

## State 2: Internal temperature display

- State 2 reads the internal STM32 temperature sensor using ADC1 channel 18.
- The ADC is configured in single-conversion mode with a long sample time.
- The temperature sensor is enabled through the ADC common control register before conversion.
- The implementation averages 32 ADC samples per update to reduce noise.
- This behavior depends on the STM32C5 MCU peripheral, not on a board-level Nucleo feature described in UM3616.
- Temperature conversion currently uses fixed STM32C5 datasheet constants:
	- `V25 = 0.76 V`
	- `Average slope = 2.5 mV/°C`
	- `VREF = 3.3 V`
- The computed temperature is clamped to the range `-40°C..85°C` before display.
- The displayed output uses one decimal place and a trailing `C` indicator.
- No VREFINT-based supply-voltage compensation is currently part of the implemented behavior.

## State 3: Counter

- State 3 displays an unsigned counter value.
- An external rotary encoder is connected as follows:
	- `SW` -> `PA2`
	- `DT` -> `PA3`
	- `CLK` -> `PC3`
- The counter starts at `0`.
- Rotating the encoder in one direction increments the counter by one step.
- Rotating the encoder in the opposite direction decrements the counter by one step.
- The counter wraps in the range `0..9999`.
- The encoder push switch on `PA2` is currently not used by the application.
- The display continuously shows the current counter value with no automatic ticking.

## Implementation notes and non-goals

- The current firmware is bare-metal and does not use HAL display or state-management helpers.
- There is no UART output or logging requirement in the implemented code.
- There is no dedicated state-entry animation or state-number presentation phase.
- The requirements in this document intentionally reflect the code as it exists now, including current display limitations.
