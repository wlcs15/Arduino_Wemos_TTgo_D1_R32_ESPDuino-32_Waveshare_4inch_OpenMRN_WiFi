#pragma once

#include <stddef.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// Load SSID + PSK from NVS (AES-256-GCM wrap). If NVS is empty and this
// build was made with wifi_secrets.env, wrap once and store in NVS.
esp_err_t wifi_cred_load(char *ssid, size_t ssid_len, char *psk, size_t psk_len);

#ifdef __cplusplus
}
#endif
