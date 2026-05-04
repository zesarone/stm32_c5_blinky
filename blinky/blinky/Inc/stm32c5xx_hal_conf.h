#ifndef STM32C5XX_HAL_CONF_H
#define STM32C5XX_HAL_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

#define USE_HAL_TICK_INT_PRIORITY              HAL_TICK_INT_LOWEST_PRIORITY
#define USE_HAL_FLASH_PREFETCH                 1U

#define USE_HAL_MUTEX                          0U
#define USE_HAL_CHECK_PARAM                    0U
#define USE_HAL_CHECK_PROCESS_STATE            0U

#define USE_HAL_CORTEX_MODULE                  1U

#define USE_HAL_ETH_MODULE                     1U
#define USE_HAL_ETH_REGISTER_CALLBACKS         0U
#define USE_HAL_ETH_CLK_ENABLE_MODEL           HAL_CLK_ENABLE_NO
#define USE_HAL_ETH_USER_DATA                  0U
#define USE_HAL_ETH_GET_LAST_ERRORS            0U
#define USE_HAL_ETH_ATOMIC_CHANNEL_LOCK        0U
#define USE_HAL_ETH_MAX_TX_CH_NB               1U
#define USE_HAL_ETH_MAX_RX_CH_NB               1U

#define USE_HAL_EXTI_MODULE                    1U
#define USE_HAL_EXTI_REGISTER_CALLBACKS        0U
#define USE_HAL_EXTI_USER_DATA                 0U

#define USE_HAL_GPIO_MODULE                    1U
#define USE_HAL_GPIO_CLK_ENABLE_MODEL          HAL_CLK_ENABLE_NO

#define USE_HAL_RCC_MODULE                     1U

#ifdef __cplusplus
}
#endif

#endif