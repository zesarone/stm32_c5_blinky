#include <stdint.h>
#include "segment_display.h"

static uint16_t counter = 0;

void state_3_run(void) {
    segment_display_show_number(counter);

    counter++;
    if (counter > 9999) {
        counter = 0;
    }
}