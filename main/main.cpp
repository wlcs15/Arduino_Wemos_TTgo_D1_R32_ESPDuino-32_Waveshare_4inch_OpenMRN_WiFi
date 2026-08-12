// Scaffold for a WiFi-only OpenMRN node on the Wemos D1 R32.
// No TWAI/CAN and no ILI9486 UI in this step.

#include <stdio.h>
#include <string.h>

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "nvs_flash.h"

static const char *TAG = "d1r32_openmrn_wifi";

#if defined(WIFI_SSID_FROM_ENV) && defined(WIFI_PASSWORD_FROM_ENV)
static const char *kWifiSsid = WIFI_SSID_FROM_ENV;
static const char *kWifiPassword = WIFI_PASSWORD_FROM_ENV;
static const bool kWifiFromEnv = true;
#else
static const char *kWifiSsid = CONFIG_NODE_WIFI_SSID;
static const char *kWifiPassword = CONFIG_NODE_WIFI_PASSWORD;
static const bool kWifiFromEnv = false;
#endif

static bool wifi_creds_ready(void)
{
    return kWifiSsid != nullptr && kWifiSsid[0] != '\0' &&
           kWifiPassword != nullptr && kWifiPassword[0] != '\0';
}

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "Arduino_Wemos_TTgo_D1_R32_ESPDuino-32_Waveshare_4inch_OpenMRN_WiFi");
    ESP_LOGI(TAG, "Phase: WiFi GridConnect scaffold (no CAN, no display driver)");
    ESP_LOGI(TAG, "OpenMRNIDF is a git submodule under components/OpenMRNIDF");
    ESP_LOGI(TAG, "Required ESP-IDF: v5.1.6  target: esp32");

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    const uint64_t node_id = CONFIG_NODE_OPENLCB_ID;
    ESP_LOGI(TAG,
             "OpenLCB node ID %02X.%02X.%02X.%02X.%02X.%02X",
             (unsigned)((node_id >> 40) & 0xFF),
             (unsigned)((node_id >> 32) & 0xFF),
             (unsigned)((node_id >> 24) & 0xFF),
             (unsigned)((node_id >> 16) & 0xFF),
             (unsigned)((node_id >> 8) & 0xFF),
             (unsigned)(node_id & 0xFF));

    ESP_LOGI(TAG, "LCC hub (CS-105) %s:%d",
             CONFIG_NODE_LCC_HUB_HOST, CONFIG_NODE_LCC_HUB_PORT);
    ESP_LOGI(TAG, "JMRI monitor (Pi) %s:%d — connect JMRI to the CS-105, not as a second hub",
             CONFIG_NODE_JMRI_MONITOR_HOST, CONFIG_NODE_JMRI_MONITOR_PORT);
    if (wifi_creds_ready())
    {
        ESP_LOGI(TAG, "WiFi credentials present (%s, SSID length %u) — not printed",
                 kWifiFromEnv ? "wifi_secrets.env" : "Kconfig",
                 (unsigned)strlen(kWifiSsid));
    }
    else
    {
        ESP_LOGW(TAG, "WiFi credentials empty. Copy wifi_secrets.env.example and use utils/build_idf5.sh");
    }
    (void)kWifiPassword;

    ESP_LOGI(TAG, "Next: wire Esp32WiFiManager + SimpleStack to the CS-105 hub.");
}
