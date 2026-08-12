// Minimal ILI9486 for D1 R32 + Waveshare 4" shield (SPI D11/D12/D13, 8 MHz).
// Command path matches the working Arduino ESP32 write16() (high byte 0).

#include "ili9486_min.h"
#include "font8x8.h"

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

static const gpio_num_t PIN_CS = GPIO_NUM_5;
static const gpio_num_t PIN_DC = GPIO_NUM_14;
static const gpio_num_t PIN_RST = GPIO_NUM_12;
static const gpio_num_t PIN_BL = GPIO_NUM_13;
static const gpio_num_t PIN_SCLK = GPIO_NUM_18;
static const gpio_num_t PIN_MOSI = GPIO_NUM_23;
static const gpio_num_t PIN_MISO = GPIO_NUM_19;

static const int kWidth = 480;
static const int kHeight = 320;

static spi_device_handle_t s_spi;
static SemaphoreHandle_t s_lock;

static void lock_bus(void)
{
    if (s_lock)
    {
        xSemaphoreTakeRecursive(s_lock, portMAX_DELAY);
    }
}

static void unlock_bus(void)
{
    if (s_lock)
    {
        xSemaphoreGiveRecursive(s_lock);
    }
}

static void wr16(uint16_t v)
{
    spi_transaction_t t = {};
    t.length = 16;
    t.flags = SPI_TRANS_USE_TXDATA;
    t.tx_data[0] = (uint8_t)(v >> 8);
    t.tx_data[1] = (uint8_t)(v & 0xFF);
    (void)spi_device_polling_transmit(s_spi, &t);
}

static void cmd(uint8_t c)
{
    gpio_set_level(PIN_DC, 0);
    wr16(c);
}

static void data8(uint8_t d)
{
    gpio_set_level(PIN_DC, 1);
    wr16(d);
}

static void set_window(int x, int y, int w, int h)
{
    const uint16_t x1 = (uint16_t)x, x2 = (uint16_t)(x + w - 1);
    const uint16_t y1 = (uint16_t)y, y2 = (uint16_t)(y + h - 1);
    uint8_t xb[8] = {0, (uint8_t)(x1 >> 8), 0, (uint8_t)(x1 & 0xFF),
                     0, (uint8_t)(x2 >> 8), 0, (uint8_t)(x2 & 0xFF)};
    uint8_t yb[8] = {0, (uint8_t)(y1 >> 8), 0, (uint8_t)(y1 & 0xFF),
                     0, (uint8_t)(y2 >> 8), 0, (uint8_t)(y2 & 0xFF)};
    cmd(0x2A);
    gpio_set_level(PIN_DC, 1);
    spi_transaction_t tx = {};
    tx.length = 64;
    tx.tx_buffer = xb;
    (void)spi_device_polling_transmit(s_spi, &tx);
    cmd(0x2B);
    gpio_set_level(PIN_DC, 1);
    tx.tx_buffer = yb;
    (void)spi_device_polling_transmit(s_spi, &tx);
}

esp_err_t ili9486_init(void)
{
    if (s_lock == nullptr)
    {
        s_lock = xSemaphoreCreateRecursiveMutex();
    }
    gpio_config_t io = {};
    io.mode = GPIO_MODE_OUTPUT;
    io.pin_bit_mask = (1ULL << PIN_CS) | (1ULL << PIN_DC) | (1ULL << PIN_RST) | (1ULL << PIN_BL);
    gpio_config(&io);
    gpio_set_level(PIN_CS, 1);
    gpio_set_level(PIN_RST, 1);
    gpio_set_level(PIN_BL, 1);

    spi_bus_config_t bus = {};
    bus.mosi_io_num = PIN_MOSI;
    bus.miso_io_num = PIN_MISO;
    bus.sclk_io_num = PIN_SCLK;
    bus.quadwp_io_num = -1;
    bus.quadhd_io_num = -1;
    bus.max_transfer_sz = 4096;
    esp_err_t err = spi_bus_initialize(SPI2_HOST, &bus, SPI_DMA_CH_AUTO);
    if (err != ESP_OK)
    {
        return err;
    }
    spi_device_interface_config_t dev = {};
    dev.clock_speed_hz = 8000000;
    dev.mode = 0;
    dev.spics_io_num = PIN_CS;
    dev.queue_size = 4;
    err = spi_bus_add_device(SPI2_HOST, &dev, &s_spi);
    if (err != ESP_OK)
    {
        return err;
    }

    gpio_set_level(PIN_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(5));
    gpio_set_level(PIN_RST, 0);
    esp_rom_delay_us(20);
    gpio_set_level(PIN_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(65));

    cmd(0xC0); data8(0x19); data8(0x1A);
    cmd(0xC1); data8(0x45); data8(0x00);
    cmd(0xC2); data8(0x33);
    cmd(0xC5); data8(0x00); data8(0x28);
    cmd(0xB1); data8(0xA0); data8(0x11);
    cmd(0xB4); data8(0x02);
    cmd(0xB6); data8(0x00); data8(0x42); data8(0x3B);
    cmd(0xE0);
    static const uint8_t e0[] = {0x1F,0x25,0x22,0x0B,0x06,0x0A,0x4E,0xC6,0x39,0x00,0x00,0x00,0x00,0x00,0x00};
    for (unsigned i = 0; i < sizeof(e0); ++i) data8(e0[i]);
    cmd(0xE1);
    static const uint8_t e1[] = {0x1F,0x3F,0x3F,0x0F,0x1F,0x0F,0x46,0x49,0x31,0x05,0x09,0x03,0x1C,0x1A,0x00};
    for (unsigned i = 0; i < sizeof(e1); ++i) data8(e1[i]);
    cmd(0x3A); data8(0x55);
    cmd(0xB6); data8(0x00); data8(0x22);
    cmd(0x36); data8(0x68); // landscape (rotation 1)
    cmd(0x11);
    vTaskDelay(pdMS_TO_TICKS(20));
    cmd(0x29);
    ili9486_fill(0x0000);
    return ESP_OK;
}

void ili9486_fill_rect(int x, int y, int w, int h, uint16_t color)
{
    if (s_spi == nullptr || w <= 0 || h <= 0)
    {
        return;
    }
    lock_bus();
    set_window(x, y, w, h);
    cmd(0x2C);
    gpio_set_level(PIN_DC, 1);
    uint32_t n = (uint32_t)w * (uint32_t)h;
    while (n--)
    {
        wr16(color);
    }
    unlock_bus();
}

void ili9486_draw_wifi_icon(ili9486_wifi_icon_t state)
{
    const int x0 = kWidth - 36;
    const int y0 = 6;
    const uint16_t black = 0x0000;
    const uint16_t dim = 0x4208;
    const uint16_t yellow = 0xFFE0;
    const uint16_t green = 0x07E0;
    const uint16_t red = 0xF800;
    uint16_t c = dim;
    if (state == ILI9486_WIFI_ICON_SEARCH)
    {
        c = yellow;
    }
    else if (state == ILI9486_WIFI_ICON_OK)
    {
        c = green;
    }
    else if (state == ILI9486_WIFI_ICON_FAIL)
    {
        c = red;
    }
    ili9486_fill_rect(x0, y0, 32, 28, black);
    if (state == ILI9486_WIFI_ICON_OFF)
    {
        ili9486_fill_rect(x0 + 14, y0 + 22, 4, 4, dim);
        return;
    }
    if (state == ILI9486_WIFI_ICON_FAIL)
    {
        ili9486_fill_rect(x0 + 6, y0 + 10, 20, 3, red);
        ili9486_fill_rect(x0 + 14, y0 + 4, 3, 16, red);
        return;
    }
    // Three rising bars. Searching lights only the lowest bar.
    ili9486_fill_rect(x0 + 4, y0 + 20, 6, 6, c);
    if (state == ILI9486_WIFI_ICON_OK || state == ILI9486_WIFI_ICON_SEARCH)
    {
        const uint16_t mid = (state == ILI9486_WIFI_ICON_OK) ? c : dim;
        ili9486_fill_rect(x0 + 13, y0 + 12, 6, 14, mid);
        ili9486_fill_rect(x0 + 22, y0 + 4, 6, 22, (state == ILI9486_WIFI_ICON_OK) ? c : dim);
    }
}

void ili9486_fill(uint16_t color)
{
    lock_bus();
    set_window(0, 0, kWidth, kHeight);
    cmd(0x2C);
    gpio_set_level(PIN_DC, 1);
    uint8_t chunk[256];
    for (int i = 0; i < 128; ++i)
    {
        chunk[i * 2] = (uint8_t)(color >> 8);
        chunk[i * 2 + 1] = (uint8_t)(color & 0xFF);
    }
    uint32_t left = (uint32_t)kWidth * (uint32_t)kHeight;
    while (left)
    {
        const uint32_t n = left > 128 ? 128 : left;
        spi_transaction_t t = {};
        t.length = n * 16;
        t.tx_buffer = chunk;
        (void)spi_device_polling_transmit(s_spi, &t);
        left -= n;
    }
    unlock_bus();
}

static const uint8_t *glyph(char c)
{
    unsigned u = (unsigned char)c;
    if (u >= 'a' && u <= 'z')
    {
        u = (unsigned)(u - 'a' + 'A');
    }
    if (u < 0x20 || u > 0x5F)
    {
        u = '?';
    }
    return FONT8X8_20_5F[u - 0x20];
}

void ili9486_draw_text(int x, int y, const char *s, uint16_t fg, uint16_t bg, int scale)
{
    if (scale < 1)
    {
        scale = 1;
    }
    lock_bus();
    for (; *s; ++s)
    {
        const uint8_t *g = glyph(*s);
        for (int col = 0; col < 8; ++col)
        {
            const uint8_t bits = g[col];
            for (int row = 0; row < 8; ++row)
            {
                const uint16_t pix = (bits & (1u << row)) ? fg : bg;
                const int px = x + col * scale;
                const int py = y + row * scale;
                set_window(px, py, scale, scale);
                cmd(0x2C);
                gpio_set_level(PIN_DC, 1);
                for (int i = 0; i < scale * scale; ++i)
                {
                    wr16(pix);
                }
            }
        }
        x += 8 * scale;
    }
    unlock_bus();
}
