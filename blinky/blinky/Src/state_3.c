#include <stdint.h>
#include "segment_display.h"

static uint16_t counter = 0u;

void state_3_run(void)
{
    segment_display_show_number(counter);

    if (counter >= 9999u) {
        counter = 0u;
    } else {
        counter++;
    }
}