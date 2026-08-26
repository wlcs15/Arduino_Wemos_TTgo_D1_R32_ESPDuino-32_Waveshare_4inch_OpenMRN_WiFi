#include "svc_reach.h"

#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ili9486_min.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "sdkconfig.h"
#include "wifi_sta.h"

static const char *TAG = "svc_reach";

static bool tcp_is_open(const char *host, int port, int timeout_ms)
{
    if (host == nullptr || host[0] == '\0' || port <= 0)
    {
        return false;
    }

    char port_s[16];
    snprintf(port_s, sizeof(port_s), "%d", port);

    struct addrinfo hints = {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo *res = nullptr;
    if (getaddrinfo(host, port_s, &hints, &res) != 0 || res == nullptr)
    {
        return false;
    }

    int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd < 0)
    {
        freeaddrinfo(res);
        return false;
    }

    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

    const int rc = connect(fd, res->ai_addr, res->ai_addrlen);
    close(fd);
    freeaddrinfo(res);
    return rc == 0;
}

static void apply_icon(ili9486_svc_icon_id_t id, bool up)
{
    ili9486_draw_svc_icon(id, up ? ILI9486_SVC_ICON_OK : ILI9486_SVC_ICON_FAIL);
}

static void probe_task(void *arg)
{
    (void)arg;
    const char *web_host = CONFIG_NODE_JMRI_MONITOR_HOST;
    const char *lcc_host = CONFIG_NODE_LCC_HUB_HOST;
    const char *lcc_host2 = CONFIG_NODE_LCC_HUB_HOST2;
    const int web_port = CONFIG_NODE_JMRI_WEB_PORT;
    const int lcc_port = CONFIG_NODE_LCC_HUB_PORT;

    ili9486_draw_svc_icon(ILI9486_SVC_ICON_JMRI, ILI9486_SVC_ICON_OFF);
    ili9486_draw_svc_icon(ILI9486_SVC_ICON_LCC, ILI9486_SVC_ICON_OFF);

    for (;;)
    {
        if (wifi_sta_state() != WIFI_STA_CONNECTED)
        {
            apply_icon(ILI9486_SVC_ICON_JMRI, false);
            apply_icon(ILI9486_SVC_ICON_LCC, false);
            vTaskDelay(pdMS_TO_TICKS(2000));
            continue;
        }

        const bool web = tcp_is_open(web_host, web_port, 1500);
        ESP_LOGI(TAG, "JMRI web %s:%d %s", web_host, web_port, web ? "up" : "down");
        apply_icon(ILI9486_SVC_ICON_JMRI, web);

        bool lcc = tcp_is_open(lcc_host, lcc_port, 1500);
        if (!lcc && lcc_host2[0] != '\0')
        {
            lcc = tcp_is_open(lcc_host2, lcc_port, 1500);
        }
        ESP_LOGI(TAG, "LCC %s:%d %s", lcc_host, lcc_port, lcc ? "up" : "down");
        apply_icon(ILI9486_SVC_ICON_LCC, lcc);

        vTaskDelay(pdMS_TO_TICKS(8000));
    }
}

void svc_reach_start(void)
{
    static bool started = false;
    if (started)
    {
        return;
    }
    started = true;
    xTaskCreate(probe_task, "svc_reach", 4096, nullptr, 4, nullptr);
}
