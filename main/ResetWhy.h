#ifndef D1R32_RESET_WHY_H
#define D1R32_RESET_WHY_H

#ifdef __cplusplus
extern "C" {
#endif

/* Matches esp_reset_reason_t in ESP-IDF 5.1 (esp_system.h). */
enum
{
    D1R32_RST_UNKNOWN = 0,
    D1R32_RST_POWERON = 1,
    D1R32_RST_EXT = 2,
    D1R32_RST_SW = 3,
    D1R32_RST_PANIC = 4,
    D1R32_RST_INT_WDT = 5,
    D1R32_RST_TASK_WDT = 6,
    D1R32_RST_WDT = 7,
    D1R32_RST_DEEPSLEEP = 8,
    D1R32_RST_BROWNOUT = 9,
    D1R32_RST_SDIO = 10
};

static const char *d1r32_reset_why(int rr)
{
    switch (rr)
    {
    case D1R32_RST_POWERON:
        return "poweron";
    case D1R32_RST_SW:
        return "sw";
    case D1R32_RST_PANIC:
        return "panic";
    case D1R32_RST_INT_WDT:
        return "int_wdt";
    case D1R32_RST_TASK_WDT:
        return "task_wdt";
    case D1R32_RST_WDT:
        return "wdt";
    case D1R32_RST_BROWNOUT:
        return "brownout";
    default:
        return "other";
    }
}

#ifdef __cplusplus
}
#endif

#endif
