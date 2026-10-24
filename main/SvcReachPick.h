#ifndef D1R32_SVC_REACH_PICK_H
#define D1R32_SVC_REACH_PICK_H

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* LCC glass icon: real GridConnect attach wins. Static IPv4 is optional. */
static int svc_reach_lcc_icon_ok(int uplink_attached, int static_tcp_ok)
{
    if (uplink_attached)
    {
        return 1;
    }
    return static_tcp_ok ? 1 : 0;
}

static int svc_reach_host_usable(const char *host)
{
    if (host == 0 || host[0] == '\0')
    {
        return 0;
    }
    if (strncmp(host, "127.", 4) == 0 || strncmp(host, "0.", 2) == 0)
    {
        return 0;
    }
    if (strncmp(host, "169.254.", 8) == 0)
    {
        return 0;
    }
    return 1;
}

/* Hub IPv4 first (mDNS peer), then Kconfig monitor host if different. */
static int svc_reach_web_hosts(const char *hub_ip, const char *monitor,
                               char out[][16], int maxn)
{
    int n = 0;
    if (out == 0 || maxn <= 0)
    {
        return 0;
    }
    if (svc_reach_host_usable(hub_ip) && n < maxn)
    {
        strncpy(out[n], hub_ip, 15);
        out[n][15] = '\0';
        n++;
    }
    if (svc_reach_host_usable(monitor) && n < maxn)
    {
        if (n == 0 || strcmp(out[0], monitor) != 0)
        {
            strncpy(out[n], monitor, 15);
            out[n][15] = '\0';
            n++;
        }
    }
    return n;
}

#ifdef __cplusplus
}
#endif

#endif
