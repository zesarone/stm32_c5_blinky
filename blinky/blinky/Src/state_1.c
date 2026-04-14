#include <stdint.h>
#include "segment_display.h"

#define MAX_SIEVE 10000u

static uint8_t sieve[MAX_SIEVE];
static uint16_t current_candidate = 2u;
static uint8_t sieve_initialized = 0u;

static void init_sieve(void)
{
    for (uint16_t index = 0u; index < MAX_SIEVE; ++index) {
        sieve[index] = 1u;
    }

    sieve[0] = 0u;
    sieve[1] = 0u;
    current_candidate = 2u;
    sieve_initialized = 1u;
}

static uint16_t find_next_prime(void)
{
    for (;;) {
        while (current_candidate < MAX_SIEVE) {
            if (sieve[current_candidate] != 0u) {
                uint16_t prime = current_candidate;
                current_candidate++;

                for (uint32_t multiple = (uint32_t)prime * 2u; multiple < MAX_SIEVE; multiple += prime) {
                    sieve[multiple] = 0u;
                }

                return prime;
            }

            current_candidate++;
        }

        init_sieve();
    }
}

void state_1_run(void)
{
    if (!sieve_initialized) {
        init_sieve();
    }

    segment_display_show_number(find_next_prime());
}