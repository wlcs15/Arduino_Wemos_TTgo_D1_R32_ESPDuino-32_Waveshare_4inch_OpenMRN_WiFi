#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// OpenMRN GridConnect TCP client to the JMRI hub. Call after Wi-Fi is up.
// Does not own STA (wifi_sta stays). Does not erase NVS.
void lcc_uplink_start(void);

#ifdef __cplusplus
}
#endif
