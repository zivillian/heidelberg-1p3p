#include "ethernet_jl1101.h"

#include <Arduino.h>

#include "config.h"
#include "debug.h"

extern "C" {
#include "esp_eth.h"
#include "esp_eth_mac.h"
#include "esp_eth_netif_glue.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "lwip/ip4_addr.h"
}

#include "esp_eth_phy_jl1101.h"

#ifdef BOARD_DINGTIAN

static esp_eth_handle_t s_eth_handle = NULL;
static esp_netif_t *s_eth_netif = NULL;
static volatile bool s_eth_got_ip = false;

static void onEthEvent(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    (void) arg;
    (void) event_base;
    (void) event_data;

    switch (event_id) {
        case ETHERNET_EVENT_START:
            dbgln("[eth] start");
            break;
        case ETHERNET_EVENT_STOP:
            dbgln("[eth] stop");
            break;
        case ETHERNET_EVENT_CONNECTED: {
            uint8_t mac[6] = {0};
            if (s_eth_handle) {
                esp_eth_ioctl(s_eth_handle, ETH_CMD_G_MAC_ADDR, mac);
            }
            char mac_str[18];
            snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x:%02x:%02x:%02x",
                     mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            dbgln(String("[eth] link up, mac: ") + mac_str);
            break;
        }
        case ETHERNET_EVENT_DISCONNECTED:
            dbgln("[eth] link down");
            break;
        default:
            break;
    }
}

static void onGotIp(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    (void) arg;
    (void) event_base;
    (void) event_id;

    const ip_event_got_ip_t *event = (const ip_event_got_ip_t *) event_data;
    char ip_str[16];
    snprintf(ip_str, sizeof(ip_str), IPSTR, IP2STR(&event->ip_info.ip));
    s_eth_got_ip = true;
    dbgln(String("[eth] got ip: ") + ip_str);
}

bool ethernetHasIp()
{
    return s_eth_got_ip;
}

bool ethernetWaitForIp(uint32_t timeout_ms)
{
    const uint32_t start = millis();
    while (!s_eth_got_ip && (millis() - start) < timeout_ms) {
        delay(50);
    }
    return s_eth_got_ip;
}

bool setupEthernet()
{
    if (s_eth_handle != NULL) {
        return true;
    }

    s_eth_got_ip = false;

    esp_err_t err = esp_netif_init();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        dbgln("[eth] esp_netif_init failed");
        return false;
    }

    err = esp_event_loop_create_default();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        dbgln("[eth] event loop create failed");
        return false;
    }

    esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, &onEthEvent, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &onGotIp, NULL);

    esp_netif_config_t cfg = ESP_NETIF_DEFAULT_ETH();
    s_eth_netif = esp_netif_new(&cfg);
    if (s_eth_netif == NULL) {
        dbgln("[eth] netif create failed");
        return false;
    }

    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
    // JL1101 on Dingtian boards often sits at PHY addr 0
    phy_config.phy_addr = 0;
    phy_config.reset_gpio_num = PIN_ETH_PWR;

    mac_config.smi_mdc_gpio_num = PIN_ETH_MDC;
    mac_config.smi_mdio_gpio_num = PIN_ETH_MDIO;
    mac_config.clock_config.rmii.clock_mode = EMAC_CLK_OUT;
    mac_config.clock_config.rmii.clock_gpio = EMAC_CLK_OUT_180_GPIO;

    // Ensure PHY power/reset line is asserted before init
    pinMode(PIN_ETH_PWR, OUTPUT);
    digitalWrite(PIN_ETH_PWR, HIGH);
    delay(200);

    esp_eth_mac_t *mac = esp_eth_mac_new_esp32(&mac_config);
    esp_eth_phy_t *phy = esp_eth_phy_new_jl1101(&phy_config);
    if (mac == NULL || phy == NULL) {
        dbgln("[eth] mac/phy create failed");
        return false;
    }

    esp_eth_config_t config = ETH_DEFAULT_CONFIG(mac, phy);
    err = esp_eth_driver_install(&config, &s_eth_handle);
    if (err != ESP_OK) {
        dbgln("[eth] driver install failed");
        return false;
    }

    err = esp_netif_attach(s_eth_netif, esp_eth_new_netif_glue(s_eth_handle));
    if (err != ESP_OK) {
        dbgln("[eth] netif attach failed");
        return false;
    }

    err = esp_eth_start(s_eth_handle);
    if (err != ESP_OK) {
        dbgln("[eth] start failed");
        return false;
    }

    return true;
}

#else

bool setupEthernet()
{
    return false;
}

#endif
