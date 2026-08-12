#pragma once

#include <stdbool.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#if DEBUG
// Serial + ILI9486 dump of MAC, OpenLCB node ID, flash UID. Never the PSK.
esp_err_t debug_ids_show(void);
// Extra DEBUG line: whether a PSK is present. Never prints the password.
void debug_ids_show_psk_status(bool psk_ready);
#endif

#ifdef __cplusplus
}
#endif
