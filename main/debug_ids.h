#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#if DEBUG
// Serial + ILI9486 dump of MAC, OpenLCB node ID, flash UID. Never the PSK.
esp_err_t debug_ids_show(void);
#endif

#ifdef __cplusplus
}
#endif
