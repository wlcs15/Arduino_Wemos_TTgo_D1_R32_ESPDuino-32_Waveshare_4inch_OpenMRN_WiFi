#include "debug_ids.h"

#if DEBUG

#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "ili9486_min.h"
#include "wifi_cred.h"

static const char *TAG = "debug_ids";
static bool s_panel_ok = false;

esp_err_t debug_ids_show(void)
{
    uint8_t mac[6] = {};
    uint8_t uid[8] = {};
    bool uid_ok = false;
    wifi_hw_ids_read(mac, uid, &uid_ok);
    const uint64_t node = wifi_node_id();

    char mac_s[24];
    char node_s[24];
    char uid_s[24];
    snprintf(mac_s, sizeof(mac_s), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    snprintf(node_s, sizeof(node_s), "%02X.%02X.%02X.%02X.%02X.%02X",
             (unsigned)((node >> 40) & 0xFF),
             (unsigned)((node >> 32) & 0xFF),
             (unsigned)((node >> 24) & 0xFF),
             (unsigned)((node >> 16) & 0xFF),
             (unsigned)((node >> 8) & 0xFF),
             (unsigned)(node & 0xFF));
    if (uid_ok)
    {
        snprintf(uid_s, sizeof(uid_s), "%02X%02X%02X%02X%02X%02X%02X%02X",
                 uid[0], uid[1], uid[2], uid[3], uid[4], uid[5], uid[6], uid[7]);
    }
    else
    {
        snprintf(uid_s, sizeof(uid_s), "UNAVAILABLE");
    }

    ESP_LOGI(TAG, "==== DEBUG IDs (not the WiFi PSK) ====");
    ESP_LOGI(TAG, "MAC Address: %s", mac_s);
    ESP_LOGI(TAG, "OpenLCB Node ID: %s", node_s);
    ESP_LOGI(TAG, "SPI flash unique ID: %s", uid_s);

    esp_err_t err = ili9486_init();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "ILI9486 init failed (%s)", esp_err_to_name(err));
        return err;
    }
    const uint16_t white = 0xFFFF;
    const uint16_t black = 0x0000;
    const uint16_t yellow = 0xFFE0;
    ili9486_draw_text(8, 16, "MAC ADDRESS", yellow, black, 2);
    ili9486_draw_text(8, 48, mac_s, white, black, 2);
    ili9486_draw_text(8, 96, "OPENLCB NODE ID", yellow, black, 2);
    ili9486_draw_text(8, 128, node_s, white, black, 2);
    ili9486_draw_text(8, 176, "SPI FLASH UNIQUE ID", yellow, black, 2);
    ili9486_draw_text(8, 208, uid_s, white, black, 2);
    s_panel_ok = true;
    return ESP_OK;
}

void debug_ids_show_psk_status(bool psk_ready)
{
    if (psk_ready)
    {
        ESP_LOGI(TAG, "WiFi password: set (not logged)");
    }
    else
    {
        ESP_LOGW(TAG, "WiFi password: NOT SET");
    }
    if (!s_panel_ok)
    {
        return;
    }
    const uint16_t black = 0x0000;
    const uint16_t yellow = 0xFFE0;
    const uint16_t red = 0xF800;
    const uint16_t green = 0x07E0;
    ili9486_draw_text(8, 256, "WIFI PASSWORD", yellow, black, 2);
    if (psk_ready)
    {
        ili9486_draw_text(8, 288, "SET (NOT SHOWN)", green, black, 2);
    }
    else
    {
        ili9486_draw_text(8, 288, "NOT SET", red, black, 2);
    }
}

#endif
