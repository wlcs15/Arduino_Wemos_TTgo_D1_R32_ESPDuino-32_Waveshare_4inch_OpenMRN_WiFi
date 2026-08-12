#pragma once

#include "esp_err.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// D1 R32 + Waveshare 4" shield, SPI on D11/D12/D13, 8 MHz.
esp_err_t ili9486_init(void);
void ili9486_fill(uint16_t color);
void ili9486_draw_text(int x, int y, const char *s, uint16_t fg, uint16_t bg, int scale);

#ifdef __cplusplus
}
#endif
