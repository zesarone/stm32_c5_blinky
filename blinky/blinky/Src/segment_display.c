#include "segment_display.h"
#include "stm32c5a3xx.h"

// Segment pins configuration
static const struct {
    GPIO_TypeDef *port;
    uint32_t pin;
} segments[] = {
    {SEG_A_PORT, SEG_A_PIN},  // A
    {SEG_B_PORT, SEG_B_PIN},  // B
    {SEG_C_PORT, SEG_C_PIN},  // C
    {SEG_D_PORT, SEG_D_PIN},  // D
    {SEG_E_PORT, SEG_E_PIN},  // E
    {SEG_F_PORT, SEG_F_PIN},  // F
    {SEG_G_PORT, SEG_G_PIN},  // G
    {SEG_DP_PORT, SEG_DP_PIN} // DP
};

// Digit select pins
static const struct {
    GPIO_TypeDef *port;
    uint32_t pin;
} digits[] = {
    {DIGIT1_PORT, DIGIT1_PIN}, // Digit 1 (rightmost)
    {DIGIT2_PORT, DIGIT2_PIN}, // Digit 2
    {DIGIT3_PORT, DIGIT3_PIN}, // Digit 3
    {DIGIT4_PORT, DIGIT4_PIN}  // Digit 4 (leftmost)
};

static void set_pin_output(GPIO_TypeDef *port, uint32_t pin) {
    uint32_t shift = pin * 2;
    port->MODER &= ~(3U << shift);
    port->MODER |=  (1U << shift);
}

static void pin_high(GPIO_TypeDef *port, uint32_t pin) {
    port->ODR |= (1U << pin);
}

static void pin_low(GPIO_TypeDef *port, uint32_t pin) {
    port->ODR &= ~(1U << pin);
}

void segment_display_init(void) {
    // Enable GPIO clocks
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN |
                    RCC_AHB2ENR_GPIODEN | RCC_AHB2ENR_GPIOEEN | RCC_AHB2ENR_GPIOFEN;

    // Configure segment pins as outputs
    for (uint8_t i = 0; i < 8; i++) {
        set_pin_output(segments[i].port, segments[i].pin);
        pin_low(segments[i].port, segments[i].pin); // Start low
    }

    // Configure digit select pins as outputs
    for (uint8_t i = 0; i < DIGIT_COUNT; i++) {
        set_pin_output(digits[i].port, digits[i].pin);
        pin_high(digits[i].port, digits[i].pin); // Start high (digits off)
    }
}

void segment_display_clear(void) {
    // Turn off all segments
    for (uint8_t i = 0; i < 8; i++) {
        pin_low(segments[i].port, segments[i].pin);
    }
    // Turn off all digits
    for (uint8_t i = 0; i < DIGIT_COUNT; i++) {
        pin_high(digits[i].port, digits[i].pin);
    }
}

static void display_digit(uint8_t digit_pattern, uint8_t position, uint8_t dp) {
    // Turn off all digits first
    for (uint8_t i = 0; i < DIGIT_COUNT; i++) {
        pin_high(digits[i].port, digits[i].pin);
    }

    // Set segments
    for (uint8_t i = 0; i < 7; i++) { // Skip DP for now
        if (digit_pattern & (1 << i)) {
            pin_high(segments[i].port, segments[i].pin);
        } else {
            pin_low(segments[i].port, segments[i].pin);
        }
    }

    // Set decimal point
    if (dp) {
        pin_high(segments[7].port, segments[7].pin);
    } else {
        pin_low(segments[7].port, segments[7].pin);
    }

    // Enable the digit
    pin_low(digits[position].port, digits[position].pin);
}

void segment_display_show_number(uint16_t number) {
    static uint8_t current_digit = 0;
    static uint8_t digits_to_show[4] = {0, 0, 0, 0};

    // Update digits only when called with a new number
    static uint16_t last_number = 0xFFFF;
    if (number != last_number) {
        last_number = number;
        digits_to_show[0] = number % 10;        // Units
        digits_to_show[1] = (number / 10) % 10; // Tens
        digits_to_show[2] = (number / 100) % 10; // Hundreds
        digits_to_show[3] = (number / 1000) % 10; // Thousands
    }

    // Map to segment patterns
    uint8_t patterns[4];
    for (uint8_t i = 0; i < 4; i++) {
        switch (digits_to_show[i]) {
            case 0: patterns[i] = SEG_0; break;
            case 1: patterns[i] = SEG_1; break;
            case 2: patterns[i] = SEG_2; break;
            case 3: patterns[i] = SEG_3; break;
            case 4: patterns[i] = SEG_4; break;
            case 5: patterns[i] = SEG_5; break;
            case 6: patterns[i] = SEG_6; break;
            case 7: patterns[i] = SEG_7; break;
            case 8: patterns[i] = SEG_8; break;
            case 9: patterns[i] = SEG_9; break;
            default: patterns[i] = 0; break;
        }
    }

    // Display current digit
    display_digit(patterns[current_digit], current_digit, 0);

    // Move to next digit
    current_digit = (current_digit + 1) % 4;
}

void segment_display_show_float(float value) {
    // Convert float to displayable format
    // For simplicity, show as integer part with one decimal
    int32_t int_part = (int32_t)value;
    int32_t frac_part = (int32_t)((value - int_part) * 10);

    if (int_part > 999) {
        segment_display_show_number(9999); // Overflow
    } else if (int_part >= 100) {
        // Show XXX.Y format
        uint16_t display_val = int_part * 10 + frac_part;
        segment_display_show_number(display_val);
        // Note: decimal point position would need more complex multiplexing
    } else {
        // Show XX.Y format
        uint16_t display_val = int_part * 10 + frac_part;
        segment_display_show_number(display_val);
    }
}

void segment_display_show_digit(uint8_t digit, uint8_t position, uint8_t dp) {
    uint8_t pattern;
    switch (digit) {
        case 0: pattern = SEG_0; break;
        case 1: pattern = SEG_1; break;
        case 2: pattern = SEG_2; break;
        case 3: pattern = SEG_3; break;
        case 4: pattern = SEG_4; break;
        case 5: pattern = SEG_5; break;
        case 6: pattern = SEG_6; break;
        case 7: pattern = SEG_7; break;
        case 8: pattern = SEG_8; break;
        case 9: pattern = SEG_9; break;
        default: pattern = 0; break;
    }

    display_digit(pattern, position, dp);
}