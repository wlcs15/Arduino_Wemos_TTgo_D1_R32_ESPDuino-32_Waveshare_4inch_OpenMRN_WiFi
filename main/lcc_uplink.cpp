#include "lcc_uplink.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "executor/Notifiable.hxx"

#include "esp_log.h"
#include "esp_netif.h"
#include "esp_vfs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mdns.h"

#include "GitVersion.h"
#include "lcc_config.hxx"
#include "openlcb/SimpleNodeInfoDefs.hxx"
#include "openlcb/SimpleStack.hxx"
#include "sdkconfig.h"
#include "utils/ClientConnection.hxx"
#include "utils/ConfigUpdateListener.hxx"
#include "utils/GridConnectHub.hxx"
#include "utils/socket_listener.hxx"
#include "wifi_sta.h"

static const char *TAG = "lcc_uplink";

static uint8_t s_cfg[1024];
static off_t s_off;

static int ram_open(const char *path, int flags, int mode)
{
    (void)path;
    (void)flags;
    (void)mode;
    s_off = 0;
    return 1;
}

static int ram_close(int fd)
{
    (void)fd;
    return 0;
}

static ssize_t ram_write(int fd, const void *data, size_t size)
{
    (void)fd;
    if (s_off < 0 || (size_t)s_off >= sizeof(s_cfg))
    {
        return 0;
    }
    if (s_off + (off_t)size > (off_t)sizeof(s_cfg))
    {
        size = sizeof(s_cfg) - (size_t)s_off;
    }
    memcpy(s_cfg + s_off, data, size);
    s_off += (off_t)size;
    return (ssize_t)size;
}

static ssize_t ram_read(int fd, void *dst, size_t size)
{
    (void)fd;
    if (s_off < 0 || (size_t)s_off >= sizeof(s_cfg))
    {
        return 0;
    }
    if (s_off + (off_t)size > (off_t)sizeof(s_cfg))
    {
        size = sizeof(s_cfg) - (size_t)s_off;
    }
    memcpy(dst, s_cfg + s_off, size);
    s_off += (off_t)size;
    return (ssize_t)size;
}

static off_t ram_lseek(int fd, off_t offset, int whence)
{
    (void)fd;
    off_t next = s_off;
    if (whence == SEEK_SET)
    {
        next = offset;
    }
    else if (whence == SEEK_CUR)
    {
        next += offset;
    }
    else if (whence == SEEK_END)
    {
        next = (off_t)sizeof(s_cfg) + offset;
    }
    if (next < 0 || (size_t)next > sizeof(s_cfg))
    {
        return -1;
    }
    s_off = next;
    return s_off;
}

static int ram_fstat(int fd, struct stat *st)
{
    (void)fd;
    memset(st, 0, sizeof(*st));
    st->st_mode = S_IFREG | 0666;
    st->st_size = (off_t)sizeof(s_cfg);
    return 0;
}

static void ramcfg_mount(void)
{
    static bool mounted = false;
    if (mounted)
    {
        return;
    }
    esp_vfs_t vfs = {};
    vfs.flags = ESP_VFS_FLAG_DEFAULT;
    vfs.open = ram_open;
    vfs.close = ram_close;
    vfs.write = ram_write;
    vfs.read = ram_read;
    vfs.lseek = ram_lseek;
    vfs.fstat = ram_fstat;
    ESP_ERROR_CHECK(esp_vfs_register("/ramcfg", &vfs, nullptr));
    mounted = true;
}

static constexpr openlcb::ConfigDef cfg(0);

class FactoryResetHelper : public DefaultConfigUpdateListener
{
public:
    UpdateAction apply_configuration(int fd, bool initial_load,
                                     BarrierNotifiable *done) OVERRIDE
    {
        AutoNotify n(done);
        (void)fd;
        (void)initial_load;
        return UPDATED;
    }

    void factory_reset(int fd) override
    {
        cfg.userinfo().name().write(fd, openlcb::SNIP_STATIC_DATA.model_name);
        cfg.userinfo().description().write(fd, "OwlThree D1 R32 Wi-Fi");
    }
};

static FactoryResetHelper *s_reset_helper;

namespace openlcb
{
const char *const CONFIG_FILENAME = "/ramcfg/openlcb";
const size_t CONFIG_FILE_SIZE = ConfigDef::size() + 128;
const char *const SNIP_DYNAMIC_FILENAME = CONFIG_FILENAME;
const char CDI_DATA[] =
    R"xmldata(<?xml version="1.0"?>
<cdi xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="http://openlcb.org/schema/cdi/1/1/cdi.xsd">
<identification>
  <manufacturer>OwlThree</manufacturer>
  <model>RR D1 R32 OpenMRN WiFi</model>
  <hardwareVersion>D1R32</hardwareVersion>
  <softwareVersion>)xmldata" RR_GIT_VERSION_STR(RR_GIT_VERSION) R"xmldata(</softwareVersion>
</identification>
<acdi/>
<segment space="251" origin="1">
  <group>
    <name>User Info</name>
    <string size="63"><name>User Name</name></string>
    <string size="64"><name>User Description</name></string>
  </group>
</segment>
</cdi>)xmldata";
} // namespace openlcb

static_assert(openlcb::CONFIG_FILE_SIZE <= sizeof(s_cfg),
              "RAM OpenLCB config buffer is too small");

static openlcb::SimpleCanStack *s_stack;
static int s_fd = -1;
static bool s_attached;
static char s_hub_ip[16];
static DeviceClosedNotify s_closed(&s_fd, "jmri-hub");

int lcc_uplink_is_attached(void)
{
    return (s_attached && s_fd >= 0) ? 1 : 0;
}

const char *lcc_uplink_hub_ip(void)
{
    return s_hub_ip;
}

static bool hub_host_skip(const char *host)
{
    if (!host || host[0] == '\0')
    {
        return true;
    }
    if (!strncmp(host, "127.", 4) || !strncmp(host, "0.", 2) ||
        !strncmp(host, "169.254.", 8))
    {
        return true;
    }
    const char *self = wifi_sta_ip();
    return self && self[0] && strcmp(host, self) == 0;
}

static int try_hub_host(const char *host, int port)
{
    if (hub_host_skip(host) || port <= 0)
    {
        return -1;
    }
    const int fd = ConnectSocket(host, port);
    if (fd >= 0)
    {
        strncpy(s_hub_ip, host, sizeof(s_hub_ip) - 1);
        s_hub_ip[sizeof(s_hub_ip) - 1] = '\0';
        ESP_LOGI(TAG, "hub %s:%d connected", host, port);
    }
    return fd;
}

static void ensure_mdns(void)
{
    static bool ready;
    if (ready)
    {
        return;
    }
    const esp_err_t err = mdns_init();
    if (err == ESP_OK || err == ESP_ERR_INVALID_STATE)
    {
        ready = true;
        return;
    }
    ESP_LOGW(TAG, "mdns_init %s", esp_err_to_name(err));
}

static int connect_mdns(void)
{
    ensure_mdns();
    mdns_result_t *results = nullptr;
    if (mdns_query_ptr("_openlcb-can", "_tcp", 3000, 8, &results) != ESP_OK ||
        !results)
    {
        ESP_LOGW(TAG, "mDNS _openlcb-can._tcp: no result");
        return -1;
    }
    int fd = -1;
    for (mdns_result_t *res = results; res && fd < 0; res = res->next)
    {
        const int port = res->port ? res->port : CONFIG_NODE_LCC_HUB_PORT;
        for (mdns_ip_addr_t *ipaddr = res->addr; ipaddr && fd < 0;
             ipaddr = ipaddr->next)
        {
            if (ipaddr->addr.type != ESP_IPADDR_TYPE_V4)
            {
                continue;
            }
            char host[16];
            snprintf(host, sizeof(host), IPSTR,
                     IP2STR(&ipaddr->addr.u_addr.ip4));
            ESP_LOGI(TAG, "mDNS _openlcb-can._tcp %s:%d (%s)", host, port,
                     res->hostname ? res->hostname : "");
            fd = try_hub_host(host, port);
        }
    }
    mdns_query_results_free(results);
    return fd;
}

static int connect_hub(void)
{
    int fd = connect_mdns();
    if (fd >= 0)
    {
        return fd;
    }
    fd = try_hub_host(CONFIG_NODE_LCC_HUB_HOST, CONFIG_NODE_LCC_HUB_PORT);
    if (fd >= 0)
    {
        return fd;
    }
    return try_hub_host(CONFIG_NODE_LCC_HUB_HOST2, CONFIG_NODE_LCC_HUB_PORT);
}

static void uplink_task(void *arg)
{
    (void)arg;
    for (;;)
    {
        if (s_attached && s_fd < 0)
        {
            s_attached = false;
            s_hub_ip[0] = '\0';
            ESP_LOGW(TAG, "hub closed, retry");
        }
        if (!s_attached && wifi_sta_state() == WIFI_STA_CONNECTED)
        {
            const int fd = connect_hub();
            if (fd >= 0)
            {
                s_attached = true;
                s_fd = fd;
                create_gc_port_for_can_hub(s_stack->can_hub(), fd, &s_closed,
                                           true);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void lcc_uplink_start(void)
{
    if (s_stack)
    {
        return;
    }

    ramcfg_mount();
    // SNIP FILE_LITERAL_BYTE HASSERTs unless ACDI version byte is 2.
    s_cfg[0] = 2;

    const uint64_t node_id = CONFIG_NODE_OPENLCB_ID;
    ESP_LOGI(TAG, "OpenMRN node %02X.%02X.%02X.%02X.%02X.%02X",
             (unsigned)((node_id >> 40) & 0xFF),
             (unsigned)((node_id >> 32) & 0xFF),
             (unsigned)((node_id >> 24) & 0xFF),
             (unsigned)((node_id >> 16) & 0xFF),
             (unsigned)((node_id >> 8) & 0xFF),
             (unsigned)(node_id & 0xFF));
    ESP_LOGI(TAG, "hub mDNS _openlcb-can._tcp, then %s / %s port %d",
             CONFIG_NODE_LCC_HUB_HOST[0] ? CONFIG_NODE_LCC_HUB_HOST : "(none)",
             CONFIG_NODE_LCC_HUB_HOST2[0] ? CONFIG_NODE_LCC_HUB_HOST2 : "(none)",
             CONFIG_NODE_LCC_HUB_PORT);

    s_stack = new openlcb::SimpleCanStack(node_id);
    // ConfigUpdateService exists only after the stack. Do not construct this
    // at file scope (static init asserts Singleton instance_ == nullptr).
    s_reset_helper = new FactoryResetHelper();
    s_stack->create_config_file_if_needed(cfg.seg().internal_config(),
                                          openlcb::CANONICAL_VERSION,
                                          openlcb::CONFIG_FILE_SIZE);
    s_stack->start_executor_thread("openmrn", 5, 8192);
    xTaskCreate(uplink_task, "lcc_uplink", 6144, nullptr, 4, nullptr);
}
