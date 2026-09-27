/*
 * Transmit test for the Waveshare RS485 CAN Shield on a D1 R32.
 * The shield is an SN65HVD230. CAN TX is header D14, CAN RX is D15.
 * On this board those pins are GPIO21 and GPIO22. 125 kbit/s.
 * Sends standard ID 0x123 once a second. Does not start Wi-Fi or OpenMRN.
 */
#include <stdio.h>

#include "driver/twai.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "tx123";

void rr_tx123_run(void)
{
    twai_general_config_t general = TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_21, GPIO_NUM_22, TWAI_MODE_NORMAL);
    twai_timing_config_t timing = TWAI_TIMING_CONFIG_125KBITS();
    twai_filter_config_t filter = TWAI_FILTER_CONFIG_ACCEPT_ALL();
    twai_message_t msg = {0};
    esp_err_t err;

    msg.identifier = 0x123;
    msg.data_length_code = 8;
    msg.data[0] = 0x00;
    msg.data[1] = 0x11;
    msg.data[2] = 0x22;
    msg.data[3] = 0x33;
    msg.data[4] = 0x44;
    msg.data[5] = 0x55;
    msg.data[6] = 0x66;
    msg.data[7] = 0x77;

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
    ESP_LOGI(TAG, "sending standard ID 0x123");
    while (1) {
        err = twai_transmit(&msg, pdMS_TO_TICKS(100));
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "sent 0x123");
            printf("TARGET sent 0x123\n");
        } else {
            ESP_LOGE(TAG, "send failed %s", esp_err_to_name(err));
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
