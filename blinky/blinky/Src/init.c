#include "init.h"
#include "led.h"
#include "segment_display.h"

void system_init(void) {
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOGEN | RCC_AHB2ENR_GPIOCEN;

    GPIOC->MODER &= ~(3U << (USER_BUTTON_PIN * 2));
    led_init();
    segment_display_init();
}
