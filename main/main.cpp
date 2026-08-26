// Scaffold for a WiFi-only OpenMRN node on the Wemos D1 R32.
// No TWAI/CAN and no ILI9486 UI in this step.

#include <stdio.h>
#include <string.h>

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "debug_ids.h"
#include "ili9486_min.h"
#include "nvs_flash.h"
#include "wifi_cred.h"
#include "wifi_sta.h"
#include "GitVersion.h"
#include "lcc_uplink.h"
#include "openlcb/SimpleNodeInfoDefs.hxx"
#include "svc_reach.h"

namespace openlcb {
extern const SimpleNodeStaticValues SNIP_STATIC_DATA = {
    4, "OwlThree", "RR D1 R32 OpenMRN WiFi", "D1R32",
    RR_GIT_VERSION_STR(RR_GIT_VERSION)};
}

static const char *TAG = "d1r32_openmrn_wifi";

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "Arduino_Wemos_TTgo_D1_R32_ESPDuino-32_Waveshare_4inch_OpenMRN_WiFi");
    ESP_LOGI(TAG, "firmware %s", RR_GIT_VERSION_STR(RR_GIT_VERSION));
    ESP_LOGI(TAG, "Phase: WiFi STA + OpenMRN GridConnect to this JMRI hub.");
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

    ESP_LOGI(TAG, "LCC hub %s / %s :%d", CONFIG_NODE_LCC_HUB_HOST,
             CONFIG_NODE_LCC_HUB_HOST2, CONFIG_NODE_LCC_HUB_PORT);
    ESP_LOGI(TAG, "JMRI web probe %s:%d", CONFIG_NODE_JMRI_MONITOR_HOST,
             CONFIG_NODE_JMRI_WEB_PORT);

#if DEBUG
    (void)debug_ids_show();
#endif
    char ssid[33] = {};
    char psk[65] = {};
    err = wifi_cred_load(ssid, sizeof(ssid), psk, sizeof(psk));
    if (err == ESP_OK)
    {
        ESP_LOGI(TAG, "WiFi SSID ready (%u chars). PSK unwrapped, not logged.",
                 (unsigned)strlen(ssid));
    }
    else
    {
        ESP_LOGW(TAG, "WiFi PSK not available (%s)", esp_err_to_name(err));
    }
#if DEBUG
    debug_ids_show_psk_status(err);
#endif
    if (err == ESP_OK)
    {
        const esp_err_t werr = wifi_sta_start(ssid, psk);
        memset(psk, 0, sizeof(psk));
        if (werr == ESP_OK)
        {
            const wifi_sta_state_t st = wifi_sta_wait(20000);
            if (st == WIFI_STA_CONNECTED)
            {
                ESP_LOGI(TAG, "WiFi link up ip=%s rssi=%d", wifi_sta_ip(), wifi_sta_rssi());
                lcc_uplink_start();
                svc_reach_start();
            }
            else
            {
                ESP_LOGW(TAG, "WiFi not associated (%s)", wifi_sta_state_name(st));
            }
        }
    }
    else
    {
        memset(psk, 0, sizeof(psk));
#if DEBUG
        ili9486_draw_wifi_icon(ILI9486_WIFI_ICON_OFF);
#endif
        ESP_LOGW(TAG, "WiFi radio not started (no usable PSK)");
    }

}
