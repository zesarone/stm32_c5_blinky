#include <stdint.h>
#include "segment_display.h"

#define MAX_SIEVE 10000
static uint8_t sieve[MAX_SIEVE];
static uint16_t current_prime_index = 0;
static uint16_t primes_found = 0;
static uint16_t prime_list[1000]; // Store found primes

static void init_sieve(void) {
    for (uint16_t i = 0; i < MAX_SIEVE; i++) {
        sieve[i] = 1; // Assume all are prime initially
    }
    sieve[0] = sieve[1] = 0; // 0 and 1 are not prime

    // Reset prime finding
    current_prime_index = 0;
    primes_found = 0;

    // Pre-compute all primes using Sieve of Eratosthenes
    for (uint16_t i = 2; i < MAX_SIEVE; i++) {
        if (sieve[i]) {
            // Found a prime
            if (primes_found < 1000) {
                prime_list[primes_found++] = i;
            }

            // Mark multiples as not prime
            for (uint16_t multiple = i * 2; multiple < MAX_SIEVE; multiple += i) {
                sieve[multiple] = 0;
            }
        }
    }
}

static uint16_t get_next_prime(void) {
    if (current_prime_index < primes_found) {
        uint16_t prime = prime_list[current_prime_index];
        current_prime_index++;
        return prime;
    } else {
        // Reset when we reach the end
        current_prime_index = 0;
        return prime_list[0];
    }
}

void state_1_run(void) {
    static uint8_t initialized = 0;

    if (!initialized) {
        init_sieve();
        initialized = 1;
    }

    uint16_t prime = get_next_prime();
    segment_display_show_number(prime);
}