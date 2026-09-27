/*
 * Receive-only check for the Waveshare RS485-CAN shield on a D1 R32.
 * Prints when standard ID 0x123 arrives. Does not start Wi-Fi or OpenMRN.
 * Shield header: SPI D11/D12/D13, CS D10, INT D2.
 * D1 R32: MOSI 23, MISO 19, SCK 18, CS GPIO5, INT GPIO26.
 * MCP2515 crystal on this shield is 8 MHz. Bit rate 125 kbit/s.
 */
#include <stdio.h>

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <stdint.h>

static const char *TAG = "rx123";
static const gpio_num_t PIN_CS = GPIO_NUM_5;
static const gpio_num_t PIN_INT = GPIO_NUM_26;
static const gpio_num_t PIN_SCLK = GPIO_NUM_18;
static const gpio_num_t PIN_MOSI = GPIO_NUM_23;
static const gpio_num_t PIN_MISO = GPIO_NUM_19;

static spi_device_handle_t s_spi;

static uint8_t mcp_read(uint8_t reg)
{
    uint8_t tx[3] = {0x03, reg, 0};
    uint8_t rx[3] = {0};
    spi_transaction_t t = {
        .length = 24,
        .tx_buffer = tx,
        .rx_buffer = rx,
    };
    spi_device_transmit(s_spi, &t);
    return rx[2];
}

static void mcp_write(uint8_t reg, uint8_t val)
{
    uint8_t tx[3] = {0x02, reg, val};
    spi_transaction_t t = {
        .length = 24,
        .tx_buffer = tx,
    };
    spi_device_transmit(s_spi, &t);
}

static void mcp_reset(void)
{
    uint8_t tx = 0xC0;
    spi_transaction_t t = {
        .length = 8,
        .tx_buffer = &tx,
    };
    spi_device_transmit(s_spi, &t);
}

static int rx_standard(uint32_t *id)
{
    uint8_t flags = mcp_read(0x2C);
    uint8_t sidh;
    uint8_t sidl;
    if ((flags & 0x01) == 0) {
        return 0;
    }
    sidh = mcp_read(0x61);
    sidl = mcp_read(0x62);
    *id = ((uint32_t)sidh << 3) | (sidl >> 5);
    mcp_write(0x2C, (uint8_t)~0x01);
    return 1;
}

void rr_rx123_run(void)
{
    spi_bus_config_t bus = {
        .mosi_io_num = PIN_MOSI,
        .miso_io_num = PIN_MISO,
        .sclk_io_num = PIN_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };
    spi_device_interface_config_t dev = {
        .clock_speed_hz = 1000000,
        .mode = 0,
        .spics_io_num = PIN_CS,
        .queue_size = 1,
    };
    uint8_t stat;

    ESP_LOGI(TAG, "RS485-CAN shield CS GPIO5 INT GPIO26 SPI 18/23/19");
    spi_bus_initialize(SPI2_HOST, &bus, SPI_DMA_DISABLED);
    spi_bus_add_device(SPI2_HOST, &dev, &s_spi);
    gpio_set_direction(PIN_INT, GPIO_MODE_INPUT);
    gpio_set_pull_mode(PIN_INT, GPIO_PULLUP_ONLY);
    mcp_reset();
    vTaskDelay(pdMS_TO_TICKS(20));
    stat = mcp_read(0x0E);
    ESP_LOGI(TAG, "after reset STAT=%02x", stat);
    mcp_write(0x2A, 0x01);
    mcp_write(0x29, 0xB1);
    mcp_write(0x28, 0x85);
    mcp_write(0x60, 0x60);
    mcp_write(0x2B, 0x00);
    mcp_write(0x0F, 0x07);
    ESP_LOGI(TAG, "listening for standard ID 0x123");
    while (1) {
        uint32_t id = 0;
        if (rx_standard(&id) && id == 0x123) {
            ESP_LOGI(TAG, "received 0x123");
            printf("TARGET received 0x123\n");
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
