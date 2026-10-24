#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// OpenMRN GridConnect TCP client to the JMRI hub. Call after Wi-Fi is up.
// Does not own STA (wifi_sta stays). Does not erase NVS.
void lcc_uplink_start(void);
/* 1 when GridConnect TCP to the JMRI hub is up (mDNS or static). */
int lcc_uplink_is_attached(void);
/* Dotted IPv4 of that hub, or empty. For the JMRI-web icon probe. */
const char *lcc_uplink_hub_ip(void);

#ifdef __cplusplus
}
#endif
