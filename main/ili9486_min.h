#pragma once

#include "esp_err.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    ILI9486_WIFI_ICON_OFF = 0,
    ILI9486_WIFI_ICON_SEARCH,
    ILI9486_WIFI_ICON_OK,
    ILI9486_WIFI_ICON_FAIL,
} ili9486_wifi_icon_t;

typedef enum
{
    ILI9486_SVC_ICON_JMRI = 0,
    ILI9486_SVC_ICON_LCC,
} ili9486_svc_icon_id_t;

typedef enum
{
    ILI9486_SVC_ICON_OFF = 0,
    ILI9486_SVC_ICON_OK,
    ILI9486_SVC_ICON_FAIL,
} ili9486_svc_icon_t;

// D1 R32 + Waveshare 4" shield, SPI on D11/D12/D13, 8 MHz.
esp_err_t ili9486_init(void);
void ili9486_fill(uint16_t color);
void ili9486_fill_rect(int x, int y, int w, int h, uint16_t color);
void ili9486_draw_text(int x, int y, const char *s, uint16_t fg, uint16_t bg, int scale);
// Right-justified strip: [LCC][JMRI][Wi-Fi bars] along the top.
void ili9486_draw_wifi_icon(ili9486_wifi_icon_t state);
void ili9486_draw_svc_icon(ili9486_svc_icon_id_t id, ili9486_svc_icon_t state);

#ifdef __cplusplus
}
#endif
