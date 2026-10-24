#pragma once

#include "esp_err.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    WIFI_STA_IDLE = 0,
    WIFI_STA_NO_PSK,
    WIFI_STA_SEARCHING,
    WIFI_STA_CONNECTED,
    WIFI_STA_FAILED,
} wifi_sta_state_t;

// Start STA. Never logs the PSK. SSID may be logged (it is public).
esp_err_t wifi_sta_start(const char *ssid, const char *psk);
wifi_sta_state_t wifi_sta_state(void);
const char *wifi_sta_state_name(wifi_sta_state_t st);
const char *wifi_sta_ip(void);
int wifi_sta_rssi(void);
// Block up to timeout_ms waiting for GOT_IP. Updates the DEBUG icon.
wifi_sta_state_t wifi_sta_wait(uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif
