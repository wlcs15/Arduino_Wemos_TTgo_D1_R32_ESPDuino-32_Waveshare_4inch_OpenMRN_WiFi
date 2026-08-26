#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Periodic TCP reachability for the JMRI web (12080) and LCC (12021) icons.
// Starts a background task. Safe to call once after Wi-Fi init.
void svc_reach_start(void);

#ifdef __cplusplus
}
#endif
