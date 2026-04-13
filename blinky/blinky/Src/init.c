#include "init.h"
#include "led.h"
#include "segment_display.h"

void system_init(void) {
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOGEN | RCC_AHB2ENR_GPIOCEN |
                    RCC_AHB2ENR_GPIOBEN | RCC_AHB2ENR_GPIODEN | RCC_AHB2ENR_GPIOEEN | RCC_AHB2ENR_GPIOFEN;

    GPIOC->MODER &= ~(3U << (USER_BUTTON_PIN * 2));

    // Set display pins as outputs
    GPIOF->MODER &= ~((3U << (13 * 2)) | (3U << (12 * 2)));
    GPIOF->MODER |= ((1U << (13 * 2)) | (1U << (12 * 2)));

    GPIOB->MODER &= ~((3U << (15 * 2)) | (3U << (14 * 2)) | (3U << (13 * 2)) | (3U << (3 * 2)));
    GPIOB->MODER |= ((1U << (15 * 2)) | (1U << (14 * 2)) | (1U << (13 * 2)) | (1U << (3 * 2)));

    GPIOE->MODER &= ~(3U << (14 * 2));
    GPIOE->MODER |= (1U << (14 * 2));

    GPIOD->MODER &= ~((3U << (6 * 2)) | (3U << (5 * 2)) | (3U << (4 * 2)) | (3U << (3 * 2)));
    GPIOD->MODER |= ((1U << (6 * 2)) | (1U << (5 * 2)) | (1U << (4 * 2)) | (1U << (3 * 2)));

    GPIOA->MODER &= ~(3U << (10 * 2));
    GPIOA->MODER |= (1U << (10 * 2));

    // Clear all display pins initially
    GPIOF->BSRR = ((1U << 13) | (1U << 12)) << 16;
    GPIOB->BSRR = ((1U << 15) | (1U << 14) | (1U << 13) | (1U << 3)) << 16;
    GPIOE->BSRR = (1U << 14) << 16;
    GPIOD->BSRR = ((1U << 6) | (1U << 5) | (1U << 4) | (1U << 3)) << 16;
    GPIOA->BSRR = (1U << 10) << 16;

    led_init();
    segment_display_init();
}
