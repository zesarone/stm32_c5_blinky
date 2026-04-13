#include <stdint.h>
#include "segment_display.h"

#define MAX_SIEVE 10000
static uint8_t sieve[MAX_SIEVE];
static uint16_t current_candidate = 2;
static uint8_t sieve_initialized = 0;

static void init_sieve(void) {
    for (uint16_t i = 0; i < MAX_SIEVE; i++) {
        sieve[i] = 1; // Assume all are prime initially
    }
    sieve[0] = sieve[1] = 0; // 0 and 1 are not prime
    current_candidate = 2;
    sieve_initialized = 1;
}

static uint16_t find_next_prime(void) {
    while (current_candidate < MAX_SIEVE) {
        if (sieve[current_candidate]) {
            // Found a prime, mark its multiples
            uint16_t prime = current_candidate;
            current_candidate++;

            // Mark multiples as not prime
            for (uint16_t multiple = prime * 2; multiple < MAX_SIEVE; multiple += prime) {
                sieve[multiple] = 0;
            }

            return prime;
        }
        current_candidate++;
    }

    // If we reach the end, reset
    init_sieve();
    return 2; // Return first prime
}

void state_1_run(void) {
    if (!sieve_initialized) {
        init_sieve();
        segment_display_show_number(2); // Start with first prime
        return;
    }

    uint16_t next_prime = find_next_prime();
    if (next_prime > 9999) {
        // Reset to first prime
        init_sieve();
        segment_display_show_number(2);
    } else {
        segment_display_show_number(next_prime);
    }
}