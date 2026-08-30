#include "CdiWellFormed.h"
#include <stdio.h>
#include <string.h>

static int g_fail;

static void expect_eq(int got, int want, const char *name)
{
    if (got != want)
    {
        printf("FAIL %s got %d want %d\n", name, got, want);
        g_fail++;
    }
}

int main(void)
{
    static const char cdi[] =
        "<?xml version=\"1.0\"?>\n"
        "<cdi xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">\n"
        "<identification>\n"
        "  <manufacturer>OwlThree</manufacturer>\n"
        "  <model>RR D1 R32 OpenMRN WiFi</model>\n"
        "</identification>\n"
        "<acdi/>\n"
        "<segment space=\"251\" origin=\"1\">\n"
        "  <group><name>User Info</name></group>\n"
        "</segment>\n"
        "</cdi>\n";
    static const char truncated[] =
        "<?xml version=\"1.0\"?><cdi><identification><manufacturer>";

    expect_eq(d1r32_cdi_configure_ready(cdi, (unsigned)strlen(cdi)), 1,
              "A5.01 full CDI");
    expect_eq(d1r32_cdi_configure_ready(truncated, (unsigned)strlen(truncated)),
              0, "truncated hides Configure");
    expect_eq(d1r32_cdi_configure_ready("", 0), 0, "empty");

    if (g_fail)
    {
        printf("%d FAIL\n", g_fail);
        return 1;
    }
    printf("host A5.01 CDI Configure OK\n");
    return 0;
}
