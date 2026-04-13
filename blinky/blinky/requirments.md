# STM32 C5 Home experiment project

This is a STM32 C5 project.

## init

all init of the board should be done in init.c

## Led state indicator

Indicate the current state by lighting the LD1, LD2 or LD3. corresponding to stat 1,2 and 3

there should be an agnostic function of lighting and turning of the leds.

LED handling should be done in led.c 


## Seven segment display

A connected KYX5461AS, 4x 7-segment with decimal point as follows:

segA is PF13
segB is pf12
segC is pb15
segD is pb14
segE is pb13
segF is pb3
segG is pe14
decimal point is pd4
digit1 is PD3
digit2 is pd6
digit3 is pd5
digit4 is PA10

Create a library file segment_display.c and initialize it.
also create an easy function that takes a float and shows it on the display.

## Changing state

state is changed by pressing the USER-button 
after changing a state show the state number on the seven segment display for 2 seconds then clear the display.

# States

all states ar implemented in each state_*.c file

## State 1
implement a prime sieve. show the latest calculated prime number on the display. while calculating the next one. when reaching a prime larger then 9999. start over

## State 2

Implement a pi calculator, start calculating to 0 decimal numbers of pi the increase the decimals by 1 for each iteration. when you have gotten a valuer of pi of any precisian 

## State 3

show a ticking number on the display counting up until 9999 then resetting to 0

## Testing

All functions should have proper unit tests. make sure the tests are valid. use mocking when needed.
make sure to cover all edge cases.

unit tests for the segment display by displaying 0000 for one second followed by 111.1 for one second and so on. 
And finally make no mistakes!