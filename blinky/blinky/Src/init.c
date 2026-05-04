#include "init.h"
#include "led.h"
#include "segment_display.h"

#define ENCODER_POLL_HZ 20000u

static void configure_input_with_pullup(GPIO_TypeDef *port, uint32_t pin)
{
    uint32_t shift = pin * 2u;

    port->MODER &= ~(3u << shift);
    port->PUPDR &= ~(3u << shift);
    port->PUPDR |= (1u << shift);
}

static void configure_encoder_systick(void)
{
    SystemCoreClockUpdate();
    SysTick_Config(SystemCoreClock / ENCODER_POLL_HZ);
}

void system_init(void) {
    // Ensure HSI is enabled
    RCC->CR1 |= RCC_CR1_HSISON;
    while ((RCC->CR1 & RCC_CR1_HSISRDY) == 0) {
        // Wait for HSI ready
    }

    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOGEN | RCC_AHB2ENR_GPIOCEN |
                    RCC_AHB2ENR_GPIOBEN | RCC_AHB2ENR_GPIODEN | RCC_AHB2ENR_GPIOEEN | RCC_AHB2ENR_GPIOFEN;

    // Enable ADC clock
    RCC->AHB2ENR |= RCC_AHB2ENR_ADC12EN;
    (void)RCC->AHB2ENR;  // Read back to ensure write completes

    // Small delay to allow GPIO clocks to stabilize
    for (volatile uint32_t i = 0; i < 100; i++) {
        __asm("nop");
    }

    GPIOC->MODER &= ~(3U << (USER_BUTTON_PIN * 2));

    configure_input_with_pullup(GPIOA, ROTARY_ENCODER_SW_PIN);
    configure_input_with_pullup(GPIOA, ROTARY_ENCODER_DT_PIN);
    configure_input_with_pullup(GPIOC, ROTARY_ENCODER_CLK_PIN);
    configure_encoder_systick();

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
