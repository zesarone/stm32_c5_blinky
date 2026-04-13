# STM32 C5 Home Experiment Project Requirements

Target board manual:
https://www.st.com/resource/en/user_manual/um3616-stm32c5-nucleo144-board-mb2310-stmicroelectronics.pdf

This document restates the original requirements in clearer language and flags the parts that are still ambiguous.

## Board initialization

- All board-level initialization must be implemented in `init.c`.
- This includes GPIO setup and any hardware initialization needed before the application state machine starts.

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
- The display must support values such as `0000` and `111.1`.

truncate overflow and only allow unsigned values


## main loop

increment the a counter starting from 0 and show it on the display then wait half a second
