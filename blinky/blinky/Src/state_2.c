#include <stdint.h>
#include "segment_display.h"
#include "stm32c5xx.h"

// STM32C5 temperature sensor implementation following datasheet specifications
// VSENSE at 25°C = 0.76V, Average slope = 2.5 mV/°C, VREF = 3.3V

// STM32C5 temperature sensor implementation following datasheet specifications
// VSENSE at 25°C = 0.76V, Average slope = 2.5 mV/°C
// Note: VREFINT correction removed as it may be causing incorrect readings

#define V25_VOLTAGE     0.76f
#define SLOPE_MV_PER_C  2.5f
#define VREF_VOLTAGE    3.3f
#define ADC_RESOLUTION  4095.0f

static float read_temperature(void) {
    // Enable ADC voltage regulator
    ADC1->CR &= ~ADC_CR_DEEPPWD;
    ADC1->CR |= ADC_CR_ADVREGEN;
    
    // Wait for regulator startup
    for (volatile uint32_t i = 0; i < 10000; ++i) {
        __asm("nop");
    }
    
    // Configure ADC for temperature sensor
    ADC1->CFGR1 = 0;
    ADC1->CFGR2 = 0;
    ADC1->SMPR1 = (7 << 24); // Sampling time for channel 18
    ADC1->SQR1 = (18 << ADC_SQR1_SQ1_Pos); // Temperature sensor on channel 18
    
    // Enable temperature sensor via common register
    ADC12_COMMON->CCR |= ADCC_CCR_TSEN;
    
    // Wait for sensor to stabilize
    for (volatile uint32_t i = 0; i < 10000; ++i) {
        __asm("nop");
    }
    
    // Enable ADC and wait for ready
    ADC1->CR |= ADC_CR_ADEN;
    for (uint32_t timeout = 100000; timeout > 0; timeout--) {
        if (ADC1->ISR & ADC_ISR_ADRDY) break;
    }
    
    // Take multiple readings and average them to reduce noise
    uint32_t adc_sum = 0;
    const uint8_t num_samples = 32;
    
    for (uint8_t sample = 0; sample < num_samples; sample++) {
        // Start conversion
        ADC1->CR |= ADC_CR_ADSTART;
        
        // Wait for conversion complete
        for (uint32_t timeout = 100000; timeout > 0; timeout--) {
            if (ADC1->ISR & ADC_ISR_EOC) break;
        }
        
        // Read result and accumulate
        adc_sum += ADC1->DR;
    }
    
    // Calculate average
    uint32_t adc_value = adc_sum / num_samples;
    
    // Disable ADC
    ADC1->CR |= ADC_CR_ADDIS;
    for (uint32_t timeout = 100000; timeout > 0; timeout--) {
        if ((ADC1->CR & ADC_CR_ADEN) == 0) break;
    }
    
    // Disable temperature sensor
    ADC12_COMMON->CCR &= ~ADCC_CCR_TSEN;
    
    // Temperature calculation using STM32C5 datasheet values
    // VSENSE = (adc_value / ADC_RESOLUTION) * VREF_VOLTAGE
    // Temperature = 25 + (V25_VOLTAGE - VSENSE) / (SLOPE_MV_PER_C / 1000)
    
    float vsense = ((float)adc_value / ADC_RESOLUTION) * VREF_VOLTAGE;
    float slope_v_per_c = SLOPE_MV_PER_C / 1000.0f;  // Convert mV/°C to V/°C
    float temperature = 25.0f + (V25_VOLTAGE - vsense) / slope_v_per_c;
    
    // Clamp to reasonable range
    if (temperature < -40.0f) temperature = -40.0f;
    if (temperature > 85.0f) temperature = 85.0f;
    
    return temperature;
}

void state_2_run(void) {
    float temperature = read_temperature();
    segment_display_show_temperature(temperature);
}
