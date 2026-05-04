#include <stdint.h>

#include "stm32_external_env.h"
#include "ethernet_diag.h"
#include "lan8742.h"
#include "segment_display.h"
#include "stm32_hal.h"

#define ETH_DIAG_DISPLAY_PHY_READY 8742u
#define ETH_DIAG_DISPLAY_LINK_10     10u
#define ETH_DIAG_DISPLAY_LINK_100   100u
#define ETH_DIAG_POLL_INTERVAL_MS   250u
#define ETH_DIAG_RETRY_INTERVAL_MS 1000u
#define ETH_DIAG_PATTERN_BLANK        0x00u

#define ETH_DIAG_ERROR_HSE_ENABLE        1011u
#define ETH_DIAG_ERROR_ETH_REF_CLK       1014u
#define ETH_DIAG_ERROR_ETH_KERNEL_CLK    1015u
#define ETH_DIAG_ERROR_HAL_INIT          1002u
#define ETH_DIAG_ERROR_HAL_CONFIG        1003u
#define ETH_DIAG_ERROR_NO_MDIO_RESPONSE  1004u
#define ETH_DIAG_ERROR_RMII_SELECT       1007u
#define ETH_DIAG_ERROR_PHY_ID_BASE       1100u
#define ETH_DIAG_ERROR_PHY_INIT_BASE     1200u
#define ETH_DIAG_ERROR_LINK_READ_BASE    1300u

typedef enum {
    ETH_DIAG_STATE_UNINITIALIZED = 0u,
    ETH_DIAG_STATE_READY = 1u,
    ETH_DIAG_STATE_FAILED = 2u
} EthernetDiagState;

static hal_eth_handle_t eth_handle;
static lan8742_obj_t phy_object;
static EthernetDiagState ethernet_diag_state = ETH_DIAG_STATE_UNINITIALIZED;
static uint32_t next_action_tick_ms = 0u;
static uint16_t display_value = 0u;
static uint8_t phy_address = 0u;

static const uint8_t eth_diag_digit_patterns[10] = {
    SEG_0,
    SEG_1,
    SEG_2,
    SEG_3,
    SEG_4,
    SEG_5,
    SEG_6,
    SEG_7,
    SEG_8,
    SEG_9,
};

static uint8_t is_lan8742_identifier(uint16_t phy_id1, uint16_t phy_id2)
{
    if ((phy_id1 & LAN8742_PHYI1R_OUI_3_18) != LAN8742_PHYI1R_OUI_3_18_DEFAULT) {
        return 0u;
    }

    if ((phy_id2 & LAN8742_PHYI2R_OUI_19_24) != LAN8742_PHYI2R_OUI_19_24_DEFAULT) {
        return 0u;
    }

    if ((phy_id2 & LAN8742_PHYI2R_MODEL_NBR) != LAN8742_PHYI2R_MODEL_NBR_DEFAULT) {
        return 0u;
    }

    return 1u;
}

static void show_all_zeroes_once(void)
{
    static const uint8_t patterns[4] = { SEG_0, SEG_0, SEG_0, SEG_0 };

    segment_display_show_raw_once(patterns, 0u);
}

static void format_number_patterns(uint16_t number, uint8_t patterns[4])
{
    uint16_t remaining = number;
    uint16_t divisors[4] = { 1000u, 100u, 10u, 1u };
    uint8_t non_zero_seen = 0u;

    if (number == 0u) {
        patterns[0] = ETH_DIAG_PATTERN_BLANK;
        patterns[1] = ETH_DIAG_PATTERN_BLANK;
        patterns[2] = ETH_DIAG_PATTERN_BLANK;
        patterns[3] = SEG_0;
        return;
    }

    for (uint8_t index = 0u; index < DIGIT_COUNT; ++index) {
        uint8_t digit = (uint8_t)(remaining / divisors[index]);
        remaining = (uint16_t)(remaining % divisors[index]);

        if ((non_zero_seen == 0u) && (digit == 0u) && (index < (DIGIT_COUNT - 1u))) {
            patterns[index] = ETH_DIAG_PATTERN_BLANK;
            continue;
        }

        non_zero_seen = 1u;
        patterns[index] = eth_diag_digit_patterns[digit];
    }
}

static void show_number_once(uint16_t number)
{
    uint8_t patterns[4];

    format_number_patterns(number, patterns);
    segment_display_show_raw_once(patterns, 0u);
}

static void show_current_display_once(void)
{
    if (display_value == 0u) {
        show_all_zeroes_once();
        return;
    }

    show_number_once(display_value);
}

static uint8_t configure_eth_kernel_clock(void)
{
    if (HAL_RCC_HSE_IsReady() != HAL_RCC_OSC_READY) {
        if (HAL_RCC_HSE_Enable(HAL_RCC_HSE_ON) != HAL_OK) {
            display_value = ETH_DIAG_ERROR_HSE_ENABLE;
            return 0u;
        }
    }

    if (HAL_RCC_ETH1REF_SetKernelClkSource(HAL_RCC_ETH1REF_CLK_SRC_RMII) != HAL_OK) {
        display_value = ETH_DIAG_ERROR_ETH_REF_CLK;
        return 0u;
    }

    if (HAL_RCC_ETH1_SetConfigKernelClk(HAL_RCC_ETH1_CLK_SRC_HSE, HAL_RCC_ETH1_PRESCALER1) != HAL_OK) {
        display_value = ETH_DIAG_ERROR_ETH_KERNEL_CLK;
        return 0u;
    }

    return 1u;
}

static uint8_t configure_eth_rmii_interface(void)
{
    HAL_RCC_SBS_EnableClock();
    __DSB();

    LL_SBS_SetETHPHYInterface(LL_SBS_PERIPH_ETH1, LL_SBS_ETHPHY_ITF_RMII);
    __DSB();

    if (LL_SBS_GetETHPHYInterface(LL_SBS_PERIPH_ETH1) != LL_SBS_ETHPHY_ITF_RMII) {
        display_value = ETH_DIAG_ERROR_RMII_SELECT;
        return 0u;
    }

    return 1u;
}

static void reset_eth1_peripheral(void)
{
    HAL_RCC_ETH1_Reset();
    HAL_RCC_ETH1_EnableClock();
    HAL_RCC_ETH1TX_EnableClock();
    HAL_RCC_ETH1RX_EnableClock();
    HAL_RCC_ETH1CK_EnableClock();
    HAL_Delay(1u);
}

static uint8_t initialize_eth_handle(void)
{
    static const hal_eth_config_t eth_config = {
        .mac_addr = { 0x02u, 0x00u, 0x00u, 0xC5u, 0xA3u, 0x01u },
        .media_interface = HAL_ETH_MEDIA_IF_RMII,
    };

    if (configure_eth_kernel_clock() == 0u) {
        return 0u;
    }

    if (configure_eth_rmii_interface() == 0u) {
        return 0u;
    }

    HAL_RCC_ETH1_EnableClock();
    HAL_RCC_ETH1TX_EnableClock();
    HAL_RCC_ETH1RX_EnableClock();
    HAL_RCC_ETH1CK_EnableClock();

    reset_eth1_peripheral();

    if (HAL_ETH_Init(&eth_handle, HAL_ETH1) != HAL_OK) {
        display_value = ETH_DIAG_ERROR_HAL_INIT;
        return 0u;
    }

    if (HAL_ETH_SetConfig(&eth_handle, &eth_config) != HAL_OK) {
        reset_eth1_peripheral();

        if (HAL_ETH_Init(&eth_handle, HAL_ETH1) != HAL_OK) {
            display_value = ETH_DIAG_ERROR_HAL_INIT;
            return 0u;
        }

        if (HAL_ETH_SetConfig(&eth_handle, &eth_config) != HAL_OK) {
            display_value = ETH_DIAG_ERROR_HAL_CONFIG;
            return 0u;
        }
    }

    return 1u;
}

static uint8_t find_phy_address(uint8_t *address_out)
{
    uint16_t phy_id1;
    uint16_t phy_id2;
    uint8_t found_any_phy_response = 0u;
    uint8_t first_responsive_address = 0u;

    for (uint32_t address = 0u; address <= LAN8742_MAX_DEV_ADDR; ++address) {
        if (HAL_ETH_MDIO_C22ReadData(&eth_handle, (uint8_t)address, LAN8742_PHYI1R, &phy_id1) != HAL_OK) {
            continue;
        }

        if (HAL_ETH_MDIO_C22ReadData(&eth_handle, (uint8_t)address, LAN8742_PHYI2R, &phy_id2) != HAL_OK) {
            continue;
        }

        if (found_any_phy_response == 0u) {
            found_any_phy_response = 1u;
            first_responsive_address = (uint8_t)address;
        }

        if (is_lan8742_identifier(phy_id1, phy_id2) != 0u) {
            *address_out = (uint8_t)address;
            return 1u;
        }
    }

    if (found_any_phy_response != 0u) {
        display_value = (uint16_t)(ETH_DIAG_ERROR_PHY_ID_BASE + first_responsive_address);
    } else {
        display_value = ETH_DIAG_ERROR_NO_MDIO_RESPONSE;
    }

    return 0u;
}

static uint8_t initialize_phy(uint8_t address)
{
    phy_object = (lan8742_obj_t){ 0 };
    phy_object.io.heth = &eth_handle;
    phy_object.io.addr = address;

    if (lan8742_init(&phy_object, 0u) != LAN8742_STATUS_OK) {
        display_value = (uint16_t)(ETH_DIAG_ERROR_PHY_INIT_BASE + address);
        return 0u;
    }

    if (lan8742_software_reset(&phy_object) != LAN8742_STATUS_OK) {
        display_value = (uint16_t)(ETH_DIAG_ERROR_PHY_INIT_BASE + address);
        return 0u;
    }

    phy_address = address;
    return 1u;
}

static void update_link_display(void)
{
    lan8742_link_t link_mode = {
        .status = LAN8742_LINK_DOWN,
        .speed = LAN8742_LINK_SPEED_NONE,
        .duplex = LAN8742_LINK_DUPLEX_NONE,
    };
    lan8742_status_t status = lan8742_get_link_mode(&phy_object, &link_mode);

    if ((status == LAN8742_STATUS_READ_ERROR) || (status == LAN8742_STATUS_ERR_INVALID_STATE)) {
        display_value = (uint16_t)(ETH_DIAG_ERROR_LINK_READ_BASE + phy_address);
        ethernet_diag_state = ETH_DIAG_STATE_FAILED;
        next_action_tick_ms = HAL_GetTick() + ETH_DIAG_RETRY_INTERVAL_MS;
        return;
    }

    if (link_mode.speed == LAN8742_LINK_SPEED_100) {
        display_value = ETH_DIAG_DISPLAY_LINK_100;
        return;
    }

    if (link_mode.speed == LAN8742_LINK_SPEED_10) {
        display_value = ETH_DIAG_DISPLAY_LINK_10;
        return;
    }

    display_value = ETH_DIAG_DISPLAY_PHY_READY;
}

static void try_initialize_diagnostics(void)
{
    uint8_t detected_phy_address;

    if (initialize_eth_handle() == 0u) {
        ethernet_diag_state = ETH_DIAG_STATE_FAILED;
        next_action_tick_ms = HAL_GetTick() + ETH_DIAG_RETRY_INTERVAL_MS;
        return;
    }

    if (find_phy_address(&detected_phy_address) == 0u) {
        ethernet_diag_state = ETH_DIAG_STATE_FAILED;
        next_action_tick_ms = HAL_GetTick() + ETH_DIAG_RETRY_INTERVAL_MS;
        return;
    }

    if (initialize_phy(detected_phy_address) == 0u) {
        ethernet_diag_state = ETH_DIAG_STATE_FAILED;
        next_action_tick_ms = HAL_GetTick() + ETH_DIAG_RETRY_INTERVAL_MS;
        return;
    }

    ethernet_diag_state = ETH_DIAG_STATE_READY;
    next_action_tick_ms = HAL_GetTick();
    display_value = ETH_DIAG_DISPLAY_PHY_READY;
}

int32_t lan8742_io_init(lan8742_io_t *pio)
{
    if ((pio == NULL) || (pio->heth == NULL)) {
        return -1;
    }

    pio->hexti = NULL;
    pio->irq_src = LAN8742_INVALID_IRQ;
    pio->it_port = (hal_gpio_t)0;
    pio->it_pin = LAN8742_INVALID_PIN;
    pio->nrst_pin = LAN8742_INVALID_PIN;
    pio->nrst_port = (hal_gpio_t)0;
    pio->nrst_active_state = (uint8_t)HAL_GPIO_PIN_RESET;

    return 0;
}

void ethernet_diag_run(void)
{
    uint32_t now = HAL_GetTick();

    if ((ethernet_diag_state != ETH_DIAG_STATE_READY) && (now >= next_action_tick_ms)) {
        try_initialize_diagnostics();
        now = HAL_GetTick();
    }

    if ((ethernet_diag_state == ETH_DIAG_STATE_READY) && (now >= next_action_tick_ms)) {
        update_link_display();
        next_action_tick_ms = now + ETH_DIAG_POLL_INTERVAL_MS;
    }

    (void)phy_address;
    show_current_display_once();
}