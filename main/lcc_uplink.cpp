#include "lcc_uplink.h"

#include <string.h>
#include <sys/stat.h>

#include "esp_log.h"
#include "esp_vfs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "openlcb/SimpleStack.hxx"
#include "sdkconfig.h"
#include "utils/ClientConnection.hxx"
#include "utils/GridConnectHub.hxx"
#include "utils/socket_listener.hxx"
#include "wifi_sta.h"

static const char *TAG = "lcc_uplink";

static uint8_t s_cfg[512];
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

namespace openlcb
{
const char *const CONFIG_FILENAME = "/ramcfg/openlcb";
const size_t CONFIG_FILE_SIZE = sizeof(s_cfg);
const char *const SNIP_DYNAMIC_FILENAME = CONFIG_FILENAME;
} // namespace openlcb

static openlcb::SimpleCanStack *s_stack;
static int s_fd = -1;
static bool s_attached;
static DeviceClosedNotify s_closed(&s_fd, "jmri-hub");

static int connect_hub(void)
{
    int fd = ConnectSocket(CONFIG_NODE_LCC_HUB_HOST, CONFIG_NODE_LCC_HUB_PORT);
    if (fd >= 0)
    {
        ESP_LOGI(TAG, "hub %s:%d connected", CONFIG_NODE_LCC_HUB_HOST,
                 CONFIG_NODE_LCC_HUB_PORT);
        return fd;
    }
    if (CONFIG_NODE_LCC_HUB_HOST2[0] == '\0')
    {
        return -1;
    }
    fd = ConnectSocket(CONFIG_NODE_LCC_HUB_HOST2, CONFIG_NODE_LCC_HUB_PORT);
    if (fd >= 0)
    {
        ESP_LOGI(TAG, "hub %s:%d connected", CONFIG_NODE_LCC_HUB_HOST2,
                 CONFIG_NODE_LCC_HUB_PORT);
    }
    return fd;
}

static void uplink_task(void *arg)
{
    (void)arg;
    for (;;)
    {
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

    const uint64_t node_id = CONFIG_NODE_OPENLCB_ID;
    ESP_LOGI(TAG, "OpenMRN node %02X.%02X.%02X.%02X.%02X.%02X",
             (unsigned)((node_id >> 40) & 0xFF),
             (unsigned)((node_id >> 32) & 0xFF),
             (unsigned)((node_id >> 24) & 0xFF),
             (unsigned)((node_id >> 16) & 0xFF),
             (unsigned)((node_id >> 8) & 0xFF),
             (unsigned)(node_id & 0xFF));
    ESP_LOGI(TAG, "hub try %s then %s port %d", CONFIG_NODE_LCC_HUB_HOST,
             CONFIG_NODE_LCC_HUB_HOST2, CONFIG_NODE_LCC_HUB_PORT);

    s_stack = new openlcb::SimpleCanStack(node_id);
    s_stack->start_executor_thread("openmrn", 5, 8192);
    xTaskCreate(uplink_task, "lcc_uplink", 4096, nullptr, 4, nullptr);
}
