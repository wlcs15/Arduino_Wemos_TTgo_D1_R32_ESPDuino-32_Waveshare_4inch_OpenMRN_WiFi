// Scaffold for a WiFi-only OpenMRN node on the Wemos D1 R32.
// No TWAI/CAN and no ILI9486 UI in this step.

#include <stdio.h>

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "nvs_flash.h"

static const char *TAG = "d1r32_openmrn_wifi";

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

    ESP_LOGI(TAG, "NVS and netif ready. Hub defaults: %s:%d",
             CONFIG_NODE_LCC_HUB_HOST, CONFIG_NODE_LCC_HUB_PORT);
    ESP_LOGI(TAG, "Next: wire Esp32WiFiManager + SimpleStack to the JMRI hub.");
}
