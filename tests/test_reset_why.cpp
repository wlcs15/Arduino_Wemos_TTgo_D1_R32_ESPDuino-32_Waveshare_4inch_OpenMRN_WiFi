#include "ResetWhy.h"
#include <stdio.h>
#include <string.h>

static int g_fail;

static void expect_str(const char *got, const char *want, const char *name)
{
    if (got == 0 || want == 0 || strcmp(got, want) != 0)
    {
        printf("FAIL %s got %s want %s\n", name, got ? got : "(null)",
               want ? want : "(null)");
        g_fail++;
    }
}

int main(void)
{
    expect_str(d1r32_reset_why(D1R32_RST_POWERON), "poweron", "poweron");
    expect_str(d1r32_reset_why(D1R32_RST_SW), "sw", "sw");
    expect_str(d1r32_reset_why(D1R32_RST_PANIC), "panic", "panic");
    expect_str(d1r32_reset_why(D1R32_RST_INT_WDT), "int_wdt", "int_wdt");
    expect_str(d1r32_reset_why(D1R32_RST_TASK_WDT), "task_wdt", "task_wdt");
    expect_str(d1r32_reset_why(D1R32_RST_WDT), "wdt", "wdt");
    expect_str(d1r32_reset_why(D1R32_RST_BROWNOUT), "brownout", "brownout");
    expect_str(d1r32_reset_why(D1R32_RST_UNKNOWN), "other", "unknown");
    expect_str(d1r32_reset_why(D1R32_RST_EXT), "other", "ext");
    expect_str(d1r32_reset_why(D1R32_RST_DEEPSLEEP), "other", "deepsleep");
    expect_str(d1r32_reset_why(D1R32_RST_SDIO), "other", "sdio");
    expect_str(d1r32_reset_why(99), "other", "out of range");

    if (g_fail)
    {
        printf("%d FAIL\n", g_fail);
        return 1;
    }
    printf("host ResetWhy OK\n");
    return 0;
}
