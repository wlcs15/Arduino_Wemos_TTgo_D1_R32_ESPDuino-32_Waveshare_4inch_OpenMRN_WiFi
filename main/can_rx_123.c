/*
 * Receive-only check for the Waveshare RS485 CAN Shield on a D1 R32.
 * The shield is an SN65HVD230. CAN TX is header D14, CAN RX is D15.
 * On this board those pins are GPIO21 and GPIO22. 125 kbit/s.
 * Prints when standard ID 0x123 arrives. Does not start Wi-Fi or OpenMRN.
 */
#include <stdio.h>

#include "driver/twai.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "rx123";

void rr_rx123_run(void)
{
    twai_general_config_t general = TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_21, GPIO_NUM_22, TWAI_MODE_NORMAL);
    twai_timing_config_t timing = TWAI_TIMING_CONFIG_125KBITS();
    twai_filter_config_t filter = TWAI_FILTER_CONFIG_ACCEPT_ALL();
    esp_err_t err;

    ESP_LOGI(TAG, "RS485-CAN shield TWAI TX GPIO21 RX GPIO22");
    err = twai_driver_install(&general, &timing, &filter);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "twai install %s", esp_err_to_name(err));
        return;
    }
    err = twai_start();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "twai start %s", esp_err_to_name(err));
        return;
    }
    ESP_LOGI(TAG, "listening for standard ID 0x123");
    while (1) {
        twai_message_t msg;
        if (twai_receive(&msg, pdMS_TO_TICKS(1000)) == ESP_OK &&
            msg.extd == 0 && msg.identifier == 0x123) {
            ESP_LOGI(TAG, "received 0x123");
            printf("TARGET received 0x123\n");
        }
    }
}
