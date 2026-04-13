#include "stm32c5xx.h"
#include "init.h"

static inline void gpio_set_output(GPIO_TypeDef *port, uint32_t pin_mask)
{
    uint32_t pin = 0;

    while (pin_mask != 0u) {
        if ((pin_mask & 1u) != 0u) {
            const uint32_t shift = pin * 2u;
            port->MODER &= ~(0x3UL << shift);
            port->MODER |= (0x1UL << shift);
            port->OSPEEDR |= (0x2UL << shift);
            port->PUPDR &= ~(0x3UL << shift);
        }
        pin_mask >>= 1u;
        pin += 1u;
    }
}

static inline void gpio_clear_output(GPIO_TypeDef *port, uint32_t pin_mask)
{
    port->ODR &= ~pin_mask;
}

void board_init(void)
{
    /* Enable GPIO clocks for ports used by the display */
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN |
                    RCC_AHB2ENR_GPIOBEN |
                    RCC_AHB2ENR_GPIODEN |
                    RCC_AHB2ENR_GPIOEEN |
                    RCC_AHB2ENR_GPIOFEN;
    (void)RCC->AHB2ENR;

    gpio_set_output(GPIOF, (1u << 13) | (1u << 12));
    gpio_set_output(GPIOB, (1u << 15) | (1u << 14) | (1u << 13) | (1u << 3));
    gpio_set_output(GPIOE, (1u << 14));
    gpio_set_output(GPIOD, (1u << 6) | (1u << 5) | (1u << 4) | (1u << 3));
    gpio_set_output(GPIOA, (1u << 10));

    gpio_clear_output(GPIOF, (1u << 13) | (1u << 12));
    gpio_clear_output(GPIOB, (1u << 15) | (1u << 14) | (1u << 13) | (1u << 3));
    gpio_clear_output(GPIOE, (1u << 14));
    gpio_clear_output(GPIOD, (1u << 6) | (1u << 5) | (1u << 4) | (1u << 3));
    gpio_clear_output(GPIOA, (1u << 10));
}

void board_delay_ms(uint32_t ms)
{
    const uint32_t ticks = SystemCoreClock / 1000U;
    if (ticks == 0U || ticks > 0xFFFFFFU) {
        while (ms--) {
            for (volatile uint32_t i = 0; i < 1000U; ++i) {
                __NOP();
            }
        }
        return;
    }

    SysTick->LOAD = ticks - 1U;
    SysTick->VAL = 0U;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;

    while (ms-- > 0U) {
        while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0U) {
            __NOP();
        }
    }

    SysTick->CTRL = 0U;
    SysTick->VAL = 0U;
}
