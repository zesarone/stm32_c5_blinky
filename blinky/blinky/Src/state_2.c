#include <stdint.h>
#include "init.h"
#include "segment_display.h"
#include "stm32c5xx.h"

#define ADC_TIMEOUT_CYCLES       100000u
#define ADC_STARTUP_DELAY_CYCLES 10000u
#define JOYSTICK_SAMPLE_COUNT    8u
#define JOYSTICK_CENTER_VALUE    2048u
#define JOYSTICK_DEAD_ZONE       512u
#define JOYSTICK_X_MID_THRESHOLD   (JOYSTICK_DEAD_ZONE + (JOYSTICK_DEAD_ZONE / 4u))
#define JOYSTICK_X_OUTER_THRESHOLD (JOYSTICK_DEAD_ZONE + (JOYSTICK_DEAD_ZONE / 2u))
#define JOYSTICK_SWITCH_HIGH_THRESHOLD ((JOYSTICK_SAMPLE_COUNT / 2u) + 1u)
#define JOYSTICK_DP_ALL_DIGITS   0x0Fu

#define JOYSTICK_X_LOW_IS_LEFT   0u
#define JOYSTICK_Y_HIGH_IS_UP    1u

#define ADC_PREPARED_ADC1        0x01u
#define ADC_PREPARED_ADC2        0x02u

#define JOYSTICK_X_ADC_CHANNEL   4u
#define JOYSTICK_Y_ADC_CHANNEL   6u

typedef enum {
    AXIS_REGION_LOW = 0u,
    AXIS_REGION_CENTER = 1u,
    AXIS_REGION_HIGH = 2u
} AxisRegion;

typedef enum {
    HORIZONTAL_LEFT_MOST = 0u,
    HORIZONTAL_LEFT_INNER = 1u,
    HORIZONTAL_CENTER_LEFT = 2u,
    HORIZONTAL_CENTER = 3u,
    HORIZONTAL_CENTER_RIGHT = 4u,
    HORIZONTAL_RIGHT_INNER = 5u,
    HORIZONTAL_RIGHT_MOST = 6u
} HorizontalPosition;

static uint8_t adc_prepared_mask = 0u;
static uint16_t last_vrx_value = JOYSTICK_CENTER_VALUE;
static uint16_t last_vry_value = JOYSTICK_CENTER_VALUE;
static uint16_t joystick_center_vrx_value = JOYSTICK_CENTER_VALUE;
static uint16_t joystick_center_vry_value = JOYSTICK_CENTER_VALUE;

#if JOYSTICK_X_LOW_IS_LEFT
#define JOYSTICK_LEFT_REGION  AXIS_REGION_LOW
#define JOYSTICK_RIGHT_REGION AXIS_REGION_HIGH
#else
#define JOYSTICK_LEFT_REGION  AXIS_REGION_HIGH
#define JOYSTICK_RIGHT_REGION AXIS_REGION_LOW
#endif

#if JOYSTICK_Y_HIGH_IS_UP
#define JOYSTICK_UP_REGION    AXIS_REGION_HIGH
#define JOYSTICK_DOWN_REGION  AXIS_REGION_LOW
#else
#define JOYSTICK_UP_REGION    AXIS_REGION_LOW
#define JOYSTICK_DOWN_REGION  AXIS_REGION_HIGH
#endif

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

static void adc_enable(ADC_TypeDef *adc)
{
    adc->ISR = ADC_ISR_ADRDY;
    adc->CR |= ADC_CR_ADEN;
    (void)wait_for_flag(&adc->ISR, ADC_ISR_ADRDY);
}

static void prepare_adc(ADC_TypeDef *adc, uint8_t mask)
{
    if ((adc_prepared_mask & mask) != 0u) {
        return;
    }

    adc->CR &= ~ADC_CR_DEEPPWD;
    adc->CR |= ADC_CR_ADVREGEN;
    delay_cycles(ADC_STARTUP_DELAY_CYCLES);

    adc->CR |= ADC_CR_ADCAL;
    while ((adc->CR & ADC_CR_ADCAL) != 0u) {
    }

    adc_enable(adc);
    adc_prepared_mask |= mask;
}

static void configure_regular_channel(ADC_TypeDef *adc, uint8_t channel)
{
    adc->CFGR1 = 0u;
    adc->CFGR2 = 0u;
    adc->SMPR1 = 0u;
    adc->SMPR2 = 0u;
    adc->PCSEL = (1u << channel);

    if (channel <= 9u) {
        adc->SMPR1 = (7u << (channel * 3u));
    } else {
        adc->SMPR2 = (7u << ((channel - 10u) * 3u));
    }

    adc->SQR1 = ((uint32_t)channel << ADC_SQR1_SQ1_Pos);
}

static uint16_t read_adc_average(ADC_TypeDef *adc, uint8_t prepared_mask, uint8_t channel, uint16_t fallback)
{
    uint32_t adc_sum = 0u;
    uint32_t collected_samples = 0u;

    prepare_adc(adc, prepared_mask);
    configure_regular_channel(adc, channel);

    for (uint32_t sample = 0u; sample < JOYSTICK_SAMPLE_COUNT; ++sample) {
        adc->ISR = ADC_ISR_EOC | ADC_ISR_EOSMP;
        adc->CR |= ADC_CR_ADSTART;

        if (wait_for_flag(&adc->ISR, ADC_ISR_EOC) == 0u) {
            break;
        }

        adc_sum += adc->DR;
        collected_samples++;
    }

    if (collected_samples == 0u) {
        return fallback;
    }

    return (uint16_t)(adc_sum / collected_samples);
}

static AxisRegion classify_axis(uint16_t value, uint16_t center_value)
{
    int32_t delta = (int32_t)value - (int32_t)center_value;

    if (delta < -(int32_t)JOYSTICK_DEAD_ZONE) {
        return AXIS_REGION_LOW;
    }

    if (delta > (int32_t)JOYSTICK_DEAD_ZONE) {
        return AXIS_REGION_HIGH;
    }

    return AXIS_REGION_CENTER;
}

static HorizontalPosition classify_horizontal_position(uint16_t value, uint16_t center_value)
{
    int32_t delta = (int32_t)value - (int32_t)center_value;

#if JOYSTICK_X_LOW_IS_LEFT
    delta = -delta;
#endif

    if (delta >= (int32_t)JOYSTICK_X_OUTER_THRESHOLD) {
        return HORIZONTAL_LEFT_MOST;
    }

    if (delta >= (int32_t)JOYSTICK_X_MID_THRESHOLD) {
        return HORIZONTAL_LEFT_INNER;
    }

    if (delta > (int32_t)JOYSTICK_DEAD_ZONE) {
        return HORIZONTAL_CENTER_LEFT;
    }

    if (delta <= -(int32_t)JOYSTICK_X_OUTER_THRESHOLD) {
        return HORIZONTAL_RIGHT_MOST;
    }

    if (delta <= -(int32_t)JOYSTICK_X_MID_THRESHOLD) {
        return HORIZONTAL_RIGHT_INNER;
    }

    if (delta < -(int32_t)JOYSTICK_DEAD_ZONE) {
        return HORIZONTAL_CENTER_RIGHT;
    }

    return HORIZONTAL_CENTER;
}

static uint8_t joystick_switch_level(void)
{
    return (JOYSTICK_SW_PORT->IDR & (1u << JOYSTICK_SW_PIN)) != 0u;
}

static uint8_t sample_switch_level(void)
{
    uint32_t high_samples = 0u;

    for (uint32_t sample = 0u; sample < JOYSTICK_SAMPLE_COUNT; ++sample) {
        high_samples += joystick_switch_level();
        delay_cycles(64u);
    }

    if (high_samples >= JOYSTICK_SWITCH_HIGH_THRESHOLD) {
        return 1u;
    }

    return 0u;
}

static uint8_t joystick_switch_is_pressed(void)
{
    return sample_switch_level() == 0u;
}

static uint8_t left_column_pattern(AxisRegion y_region)
{
    if (y_region == JOYSTICK_UP_REGION) {
        return SEG_BIT_F;
    }

    if (y_region == JOYSTICK_DOWN_REGION) {
        return SEG_BIT_E;
    }

    return SEG_BIT_E | SEG_BIT_F;
}

static uint8_t right_column_pattern(AxisRegion y_region)
{
    if (y_region == JOYSTICK_UP_REGION) {
        return SEG_BIT_B;
    }

    if (y_region == JOYSTICK_DOWN_REGION) {
        return SEG_BIT_C;
    }

    return SEG_BIT_B | SEG_BIT_C;
}

void state_2_init(void)
{
    last_vrx_value = read_adc_average(ADC1, ADC_PREPARED_ADC1, JOYSTICK_X_ADC_CHANNEL, JOYSTICK_CENTER_VALUE);
    last_vry_value = read_adc_average(ADC2, ADC_PREPARED_ADC2, JOYSTICK_Y_ADC_CHANNEL, JOYSTICK_CENTER_VALUE);

    joystick_center_vrx_value = last_vrx_value;
    joystick_center_vry_value = last_vry_value;
}

static void apply_horizontal_indicator(uint8_t patterns[4], HorizontalPosition x_position, AxisRegion y_region)
{
    uint8_t left_pattern = left_column_pattern(y_region);
    uint8_t right_pattern = right_column_pattern(y_region);

    switch (x_position) {
        case HORIZONTAL_LEFT_MOST:
            patterns[0] |= left_pattern;
            break;

        case HORIZONTAL_LEFT_INNER:
            patterns[0] |= right_pattern;
            break;

        case HORIZONTAL_CENTER_LEFT:
            patterns[1] |= left_pattern;
            break;

        case HORIZONTAL_CENTER:
            patterns[1] |= right_pattern;
            patterns[2] |= left_pattern;
            break;

        case HORIZONTAL_CENTER_RIGHT:
            patterns[2] |= right_pattern;
            break;

        case HORIZONTAL_RIGHT_INNER:
            patterns[3] |= left_pattern;
            break;

        case HORIZONTAL_RIGHT_MOST:
        default:
            patterns[3] |= right_pattern;
            break;
    }
}

static void apply_vertical_indicator(uint8_t patterns[4], HorizontalPosition x_position, uint8_t segment_bit)
{
    switch (x_position) {
        case HORIZONTAL_LEFT_MOST:
        case HORIZONTAL_LEFT_INNER:
            patterns[0] |= segment_bit;
            break;

        case HORIZONTAL_CENTER_LEFT:
            patterns[1] |= segment_bit;
            break;

        case HORIZONTAL_CENTER:
            patterns[1] |= segment_bit;
            patterns[2] |= segment_bit;
            break;

        case HORIZONTAL_CENTER_RIGHT:
            patterns[2] |= segment_bit;
            break;

        case HORIZONTAL_RIGHT_INNER:
        case HORIZONTAL_RIGHT_MOST:
        default:
            patterns[3] |= segment_bit;
            break;
    }
}

static void update_direction_patterns(uint8_t patterns[4], HorizontalPosition x_position, AxisRegion y_region)
{
    uint8_t y_center = (y_region == AXIS_REGION_CENTER);
    uint8_t up_active = (y_region == JOYSTICK_UP_REGION);
    uint8_t down_active = (y_region == JOYSTICK_DOWN_REGION);

    patterns[0] = 0u;
    patterns[1] = 0u;
    patterns[2] = 0u;
    patterns[3] = 0u;

    apply_horizontal_indicator(patterns, x_position, y_region);

    if (y_center != 0u) {
        apply_vertical_indicator(patterns, x_position, SEG_BIT_G);
    }

    if (up_active != 0u) {
        apply_vertical_indicator(patterns, x_position, SEG_BIT_A);
    }

    if (down_active != 0u) {
        apply_vertical_indicator(patterns, x_position, SEG_BIT_D);
    }
}

void state_2_run(void)
{
    uint8_t patterns[4];
    uint8_t decimal_point_mask = 0u;
    HorizontalPosition x_position;
    AxisRegion y_region;

    last_vrx_value = read_adc_average(ADC1, ADC_PREPARED_ADC1, JOYSTICK_X_ADC_CHANNEL, last_vrx_value);
    last_vry_value = read_adc_average(ADC2, ADC_PREPARED_ADC2, JOYSTICK_Y_ADC_CHANNEL, last_vry_value);

    x_position = classify_horizontal_position(last_vrx_value, joystick_center_vrx_value);
    y_region = classify_axis(last_vry_value, joystick_center_vry_value);

    update_direction_patterns(patterns, x_position, y_region);

    if (joystick_switch_is_pressed() != 0u) {
        decimal_point_mask = JOYSTICK_DP_ALL_DIGITS;
    }

    segment_display_show_raw_once(patterns, decimal_point_mask);
}
