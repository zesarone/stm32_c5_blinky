#include <stdint.h>
#include "segment_display.h"
#include "stm32c5xx.h"

#define TEMPERATURE_SAMPLE_COUNT   32u
#define ADC_TIMEOUT_CYCLES         100000u
#define ADC_STARTUP_DELAY_CYCLES   10000u
#define SENSOR_STARTUP_DELAY_CYCLES 10000u

#define V25_VOLTAGE      0.76f
#define VREF_VOLTAGE     3.3f
#define SLOPE_V_PER_C    0.0025f
#define ADC_FULL_SCALE   4095.0f

static void delay_cycles(uint32_t cycles)
{
    while (cycles-- != 0u) {
        __asm("nop");
    }
}

static uint8_t wait_for_flag(volatile uint32_t *reg, uint32_t mask)
{
    uint32_t timeout = ADC_TIMEOUT_CYCLES;

    while (timeout-- != 0u) {
        if ((*reg & mask) != 0u) {
            return 1u;
        }
    }

    return 0u;
}

static void adc_enable(void)
{
    ADC1->ISR = ADC_ISR_ADRDY;
    ADC1->CR |= ADC_CR_ADEN;
    (void)wait_for_flag(&ADC1->ISR, ADC_ISR_ADRDY);
}

static void prepare_adc(void)
{
    static uint8_t adc_prepared = 0u;

    if (adc_prepared != 0u) {
        return;
    }

    ADC1->CR &= ~ADC_CR_DEEPPWD;
    ADC1->CR |= ADC_CR_ADVREGEN;
    delay_cycles(ADC_STARTUP_DELAY_CYCLES);

    ADC1->CR |= ADC_CR_ADCAL;
    while ((ADC1->CR & ADC_CR_ADCAL) != 0u) {
    }

    adc_enable();
    adc_prepared = 1u;
}

static uint16_t read_temperature_adc(void)
{
    static uint16_t last_adc_value = 944u;
    uint32_t adc_sum = 0u;
    uint32_t collected_samples = 0u;

    prepare_adc();

    ADC1->CFGR1 = 0u;
    ADC1->CFGR2 = 0u;
    ADC1->SMPR1 = (7u << 24);
    ADC1->SQR1 = (18u << ADC_SQR1_SQ1_Pos);

    ADC12_COMMON->CCR |= ADCC_CCR_TSEN;
    delay_cycles(SENSOR_STARTUP_DELAY_CYCLES);

    for (uint32_t sample = 0u; sample < TEMPERATURE_SAMPLE_COUNT; ++sample) {
        ADC1->ISR = ADC_ISR_EOC | ADC_ISR_EOSMP;
        ADC1->CR |= ADC_CR_ADSTART;

        if (wait_for_flag(&ADC1->ISR, ADC_ISR_EOC) == 0u) {
            break;
        }

        adc_sum += ADC1->DR;
        collected_samples++;
    }

    ADC12_COMMON->CCR &= ~ADCC_CCR_TSEN;

    if (collected_samples != 0u) {
        last_adc_value = (uint16_t)(adc_sum / collected_samples);
    }

    return last_adc_value;
}

static float adc_to_temperature(uint16_t adc_value)
{
    float vsense = ((float)adc_value / ADC_FULL_SCALE) * VREF_VOLTAGE;
    float temperature = 25.0f + ((V25_VOLTAGE - vsense) / SLOPE_V_PER_C);

    if (temperature < -40.0f) {
        temperature = -40.0f;
    }

    if (temperature > 85.0f) {
        temperature = 85.0f;
    }

    return temperature;
}

void state_2_run(void)
{
    float temperature = adc_to_temperature(read_temperature_adc());
    segment_display_show_temperature(temperature);
}
