# STM32 C5 Home Experiment Project Requirements

Target board manual:
https://www.st.com/resource/en/user_manual/um3616-stm32c5-nucleo144-board-mb2310-stmicroelectronics.pdf

This document restates the original requirements in clearer language and flags the parts that are still ambiguous.

## Board initialization

- All board-level initialization must be implemented in `init.c`.
- This includes GPIO setup and any hardware initialization needed before the application state machine starts.

## LED state indicator

- The active state must be indicated with the on-board LEDs:
- State 1 uses LD1.
- State 2 uses LD2.
- State 3 uses LD3.
- LED control must be implemented in `led.c`.
- The LED module should expose hardware-agnostic functions for turning LEDs on and off.

he button transition order is `State 1 -> State 2 -> State 3 -> State 1`

## Seven-segment display

- A KYX5461AS 4-digit seven-segment display with decimal point is connected to the MCU.
- The pin mapping is:
- `segA` -> 'PF12' 
- `segB` -> 'PB3'
- `segC` -> 'PD6'
- `segD` -> 'PD4'
- `segE` -> 'PE14'
- `segF` -> 'PB15'
- `segG` -> 'PD5'
- `decimal point` -> 'PD3'
- `digit1` -> 'pf13' 
- `digit2` -> 'PB14' 
- `digit3` -> 'PB13'
- `digit4` -> 'PA10'

the segment mapping looks like this
```
Digit 1   Digit 2   Digit 3   Digit 4
|--A--|   |--A--|   |--A--|   |--A--|   
F     B   F     B   F     B   F     B   
|--G--|   |--G--|   |--G--|   |--G--|   
E     C   E     C   E     C   E     C   
|--D--| * |--D--| * |--D--| * |--D--| *   
```

- Display handling must be implemented in `segment_display.c`.
- The display module must initialize the display hardware.
- The display module must provide a simple function that accepts a floating-point value and renders it on the 4-digit display.
- The display must support values such as `   0` and `111.1`.
- do not show left-padded zeros, zero should be shown as only a sing '0' at the right most position. etc.
truncate overflow and only allow unsigned values

## State changes

- The USER button changes the active state.
- After each state change, the display must show the new state number for 2 seconds.
- After the 2-second indication, the display must be cleared before the selected state resumes normal display output.

## State implementation structure

- Each application state must be implemented in its own `state_*.c` file.

## State 1: Prime calculation

- Implement prime-number generation a strict sieve algorithm is required. 
over all process to follow:
1. start with first prime 2 and display it.
2. sieve for the next prime, when it is found, display it.
3. if new prime found > 9999 got to 1. else got to 2


## State 2: Pi calculation
Show the current temperature of the CPU in celsius.
use 1 decimal precision and end with the unit c

## State 3: Counter

- Display an increasing counter value.
- The counter must increment until it reaches `9999`.
- After `9999`, the counter must wrap back to `0`.
