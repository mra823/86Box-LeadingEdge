/*
 * 86Box    A hypervisor and IBM PC system emulator that specializes in
 *          running old operating systems and software designed for IBM
 *          PC systems and compatibles from 1981 through fairly recent
 *          system designs based on the PCI bus.
 *
 *          This file is part of the 86Box distribution.
 *
 *          Emulation of the Leading Edge Model D Real-Time Clock.
 *
 *          The Leading Edge Model D has an integrated battery-backed
 *          Real-Time Clock at I/O port 0x300-0x31F. This is a non-standard
 *          location (IBM AT uses 0x70-0x7F). The RTC is based on the
 *          National Semiconductor MM58167 chip.
 *
 *          DOS software required CLOCK.SYS or CLKDVR.SYS driver to access.
 *
 *          Note: Port 0x300 conflicts with XT-IDE, which typically uses
 *          0x300-0x30F. This is historically accurate - users had to
 *          reconfigure XT-IDE to alternate addresses.
 *
 * Authors: Matt Allen, <matt@mattallen.xyz>
 *
 *          Copyright 2025 Matt Allen.
 */
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <wchar.h>
#define HAVE_STDARG_H
#include <86box/86box.h>
#include "cpu.h"
#include <86box/timer.h>
#include <86box/machine.h>
#include <86box/io.h>
#include <86box/device.h>
#include <86box/mem.h>
#include <86box/nvr.h>
#include <86box/plat.h>
#include <86box/pic.h>

/* MM58167 RTC chip register definitions - see datasheet pg4 */
#define MM67_REGS        32

#define MM67_MSEC        0    /* milliseconds */
#define MM67_HUNTEN      1    /* hundredths/tenths of seconds */
#define MM67_SEC         2    /* seconds */
#define MM67_MIN         3    /* minutes */
#define MM67_HOUR        4    /* hours */
#define MM67_DOW         5    /* day of the week */
#define MM67_DOM         6    /* day of the month */
#define MM67_MON         7    /* month */
#define MM67_AL_MSEC     8    /* alarm: milliseconds */
#define MM67_AL_HUNTEN   9    /* alarm: hundredths/tenths of seconds */
#define MM67_AL_SEC      10   /* alarm: seconds */
#define MM67_AL_MIN      11   /* alarm: minutes */
#define MM67_AL_HOUR     12   /* alarm: hours */
#define MM67_AL_DOW      13   /* alarm: day of the week */
#define MM67_AL_DOM      14   /* alarm: day of the month (Leading Edge: YEAR) */
#define MM67_AL_MON      15   /* alarm: month */
#define MM67_AL_DONTCARE 0xc0 /* always match in compare */
#define MM67_ISTAT       16   /* IRQ status */
#define MM67_ICTRL       17   /* IRQ control */
#define MM67INT_COMPARE  0x01 /*  Compare */
#define MM67INT_TENTH    0x02 /*  Tenth */
#define MM67INT_SEC      0x04 /*  Second */
#define MM67INT_MIN      0x08 /*  Minute */
#define MM67INT_HOUR     0x10 /*  Hour */
#define MM67INT_DAY      0x20 /*  Day */
#define MM67INT_WEEK     0x40 /*  Week */
#define MM67INT_MON      0x80 /*  Month */
#define MM67_RSTCTR      18   /* reset counters */
#define MM67_RSTRAM      19   /* reset RAM */
#define MM67_STATUS      20   /* status bit */
#define MM67_GOCMD       21   /* GO Command */
#define MM67_STBYIRQ     22   /* standby IRQ */
#define MM67_TEST        31   /* test mode */

/* Leading Edge RTC device structure */
typedef struct leading_edge_rtc_t {
    nvr_t nvr;                /* NVR backend for RTC functionality */
    int8_t year_reg;          /* Register used for year storage (non-standard) */
} leading_edge_rtc_t;

/* Forward declarations */
static void mm67_time_set(nvr_t *nvr, struct tm *tm);
static void mm67_tick(nvr_t *nvr);
static void mm67_reset(nvr_t *nvr);
static void mm67_start(nvr_t *nvr);

/* Enable logging for debugging */
#define ENABLE_LEADING_EDGE_RTC_LOG 1

#ifdef ENABLE_LEADING_EDGE_RTC_LOG
int leading_edge_rtc_do_log = ENABLE_LEADING_EDGE_RTC_LOG;

static void
leading_edge_rtc_log(const char *fmt, ...)
{
    va_list ap;

    if (leading_edge_rtc_do_log) {
        va_start(ap, fmt);
        /* Also print to stderr for immediate visibility */
        vfprintf(stderr, fmt, ap);
        va_end(ap);
        va_start(ap, fmt);
        pclog_ex(fmt, ap);
        va_end(ap);
    }
}
#else
#    define leading_edge_rtc_log(fmt, ...)
#endif

/* Check if current time matches alarm time */
static int8_t
mm67_chkalrm(nvr_t *nvr, int8_t addr)
{
    return ((nvr->regs[addr - MM67_AL_SEC + MM67_SEC] == nvr->regs[addr]) || 
            ((nvr->regs[addr] & MM67_AL_DONTCARE) == MM67_AL_DONTCARE));
}

/* Get current time from RTC registers */
static void
mm67_time_get(nvr_t *nvr, struct tm *tm)
{
    const leading_edge_rtc_t *dev = (leading_edge_rtc_t *) nvr->data;
    const uint8_t *regs = nvr->regs;

    /* Read time from BCD registers */
    tm->tm_sec  = RTC_DCB(regs[MM67_SEC]);
    tm->tm_min  = RTC_DCB(regs[MM67_MIN]);
    tm->tm_hour = RTC_DCB(regs[MM67_HOUR]);
    tm->tm_wday = RTC_DCB(regs[MM67_DOW]) - 1;  /* Convert from 1-7 to 0-6 */
    tm->tm_mday = RTC_DCB(regs[MM67_DOM]);
    tm->tm_mon  = RTC_DCB(regs[MM67_MON]) - 1;  /* Convert from 1-12 to 0-11 */

    /* Get year from non-standard location (base-80 BCD) */
    if (dev->year_reg >= 0) {
        tm->tm_year = RTC_DCB(regs[dev->year_reg]) + 80;  /* Base-80: add 80 */
    }
}

/* Set RTC registers from time structure */
static void
mm67_time_set(nvr_t *nvr, struct tm *tm)
{
    const leading_edge_rtc_t *dev = (leading_edge_rtc_t *) nvr->data;
    uint8_t *regs = nvr->regs;

    /* Write time to BCD registers */
    regs[MM67_MSEC]   = 0;  /* We don't track milliseconds */
    regs[MM67_HUNTEN] = 0;  /* We don't track hundredths */
    regs[MM67_SEC]    = RTC_BCD(tm->tm_sec);
    regs[MM67_MIN]    = RTC_BCD(tm->tm_min);
    regs[MM67_HOUR]   = RTC_BCD(tm->tm_hour);
    regs[MM67_DOW]    = RTC_BCD(tm->tm_wday + 1);  /* Convert from 0-6 to 1-7 */
    regs[MM67_DOM]    = RTC_BCD(tm->tm_mday);
    regs[MM67_MON]    = RTC_BCD(tm->tm_mon + 1);   /* Convert from 0-11 to 1-12 */

    /* Store year in non-standard location (base-80 BCD) */
    if (dev->year_reg >= 0) {
        int year = tm->tm_year - 80;  /* Base-80: subtract 80 */
        regs[dev->year_reg] = RTC_BCD(year % 100);
    }
}

/*
 * RTC tick function - called every second by NVR backend
 * 
 * Increments the clock by one second and handles rollovers.
 * This is more efficient than reading system time every second.
 */
static void
mm67_tick(nvr_t *nvr)
{
    const leading_edge_rtc_t *dev = (leading_edge_rtc_t *) nvr->data;
    uint8_t *regs = nvr->regs;
    int mon, year;
    int f = 0;

    /* Increment seconds */
    regs[MM67_SEC] = RTC_BCDINC(regs[MM67_SEC], 1);
    if (regs[MM67_ICTRL] & MM67INT_SEC)
        f = MM67INT_SEC;

    /* Roll over at 60 seconds? */
    if (regs[MM67_SEC] >= RTC_BCD(60)) {
        regs[MM67_SEC] = RTC_BCD(0);
        regs[MM67_MIN] = RTC_BCDINC(regs[MM67_MIN], 1);
        if (regs[MM67_ICTRL] & MM67INT_MIN)
            f = MM67INT_MIN;

        /* Roll over at 60 minutes? */
        if (regs[MM67_MIN] >= RTC_BCD(60)) {
            regs[MM67_MIN]  = RTC_BCD(0);
            regs[MM67_HOUR] = RTC_BCDINC(regs[MM67_HOUR], 1);
            if (regs[MM67_ICTRL] & MM67INT_HOUR)
                f = MM67INT_HOUR;

            /* Roll over at 24 hours? */
            if (regs[MM67_HOUR] >= RTC_BCD(24)) {
                regs[MM67_HOUR] = RTC_BCD(0);
                regs[MM67_DOW]  = RTC_BCDINC(regs[MM67_DOW], 1);
                if (regs[MM67_ICTRL] & MM67INT_DAY)
                    f = MM67INT_DAY;

                /* Roll over day of week at 7? */
                if (regs[MM67_DOW] > RTC_BCD(7)) {
                    regs[MM67_DOW] = RTC_BCD(1);
                    if (regs[MM67_ICTRL] & MM67INT_WEEK)
                        f = MM67INT_WEEK;
                }

                /* Increment day of month */
                regs[MM67_DOM] = RTC_BCDINC(regs[MM67_DOM], 1);
                mon = RTC_DCB(regs[MM67_MON]);
                
                /* Get year for days-in-month calculation */
                if (dev->year_reg >= 0) {
                    year = RTC_DCB(regs[dev->year_reg]) + 80; /* Base-80 */
                } else {
                    year = 80;
                }
                year += 1900;

                /* Roll over at end of month? */
                if (RTC_DCB(regs[MM67_DOM]) > nvr_get_days(mon, year)) {
                    regs[MM67_DOM] = RTC_BCD(1);
                    regs[MM67_MON] = RTC_BCDINC(regs[MM67_MON], 1);
                    if (regs[MM67_ICTRL] & MM67INT_MON)
                        f = MM67INT_MON;

                    /* Roll over at 12 months? */
                    if (regs[MM67_MON] > RTC_BCD(12)) {
                        regs[MM67_MON] = RTC_BCD(1);
                        
                        /* Increment year */
                        if (dev->year_reg >= 0) {
                            regs[dev->year_reg] = RTC_BCDINC(regs[dev->year_reg], 1);
                            /* Handle century rollover (99 -> 00) */
                            if (regs[dev->year_reg] >= RTC_BCD(100))
                                regs[dev->year_reg] = RTC_BCD(0);
                        }
                    }
                }
            }
        }
    }

    /* Check for alarm match */
    if (mm67_chkalrm(nvr, MM67_AL_SEC) && mm67_chkalrm(nvr, MM67_AL_MIN) && 
        mm67_chkalrm(nvr, MM67_AL_HOUR) && mm67_chkalrm(nvr, MM67_AL_DOM) && 
        mm67_chkalrm(nvr, MM67_AL_MON)) {
        f |= MM67INT_COMPARE;
    }

    /* Set interrupt status and trigger if needed */
    if (f) {
        regs[MM67_ISTAT] |= f;
        if (nvr->irq != -1)
            picint(1 << nvr->irq);
    }
}

/* RTC reset function */
static void
mm67_reset(nvr_t *nvr)
{
    /* Clear all registers */
    memset(nvr->regs, 0x00, nvr->size);

    /* Initialize to current time */
    mm67_start(nvr);
}

/* RTC start function - initialize from system time */
static void
mm67_start(nvr_t *nvr)
{
    struct tm tm;
    leading_edge_rtc_t *dev = (leading_edge_rtc_t *) nvr->data;

    /* Initialize RTC from system time */
    nvr_time_get(&tm);
    mm67_time_set(nvr, &tm);

    leading_edge_rtc_log("Leading Edge RTC: Initialized to %04d-%02d-%02d %02d:%02d:%02d\n",
                         tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                         tm.tm_hour, tm.tm_min, tm.tm_sec);
    leading_edge_rtc_log("Leading Edge RTC: BCD registers: "
                         "sec=%02X min=%02X hour=%02X dom=%02X mon=%02X year=%02X\n",
                         nvr->regs[MM67_SEC], nvr->regs[MM67_MIN], nvr->regs[MM67_HOUR],
                         nvr->regs[MM67_DOM], nvr->regs[MM67_MON], 
                         nvr->regs[dev->year_reg]);
}

/* Read from MM58167 register */
static uint8_t
mm67_read(uint16_t port, void *priv)
{
    leading_edge_rtc_t *dev = (leading_edge_rtc_t *) priv;
    nvr_t *nvr = &dev->nvr;
    uint8_t reg = (port - 0x300) & 0x1f; /* 32 registers */
    uint8_t ret = 0xff;

    if (reg < MM67_REGS)
        ret = nvr->regs[reg];

    leading_edge_rtc_log("Leading Edge RTC: read port %04X (reg %02X) = %02X\n", 
                         port, reg, ret);

    return ret;
}

/* Write to MM58167 register */
static void
mm67_write(uint16_t port, uint8_t val, void *priv)
{
    leading_edge_rtc_t *dev = (leading_edge_rtc_t *) priv;
    nvr_t *nvr = &dev->nvr;
    uint8_t reg = (port - 0x300) & 0x1f; /* 32 registers */

    leading_edge_rtc_log("Leading Edge RTC: write port %04X (reg %02X) = %02X\n", 
                         port, reg, val);

    if (reg >= MM67_REGS)
        return;

    switch (reg) {
        case MM67_RSTCTR: /* Reset counters */
            nvr->regs[MM67_MSEC] = 0;
            nvr->regs[MM67_HUNTEN] = 0;
            break;

        case MM67_RSTRAM: /* Reset RAM */
            mm67_reset(nvr);
            break;

        case MM67_GOCMD: /* GO command - start clock */
            mm67_start(nvr);
            break;

        case MM67_ISTAT: /* Interrupt status - clear on write */
            nvr->regs[reg] = 0;
            break;

        case MM67_ICTRL: /* Interrupt control */
            nvr->regs[reg] = val;
            break;

        default:
            /* Write to register */
            nvr->regs[reg] = val;
            break;
    }
}

/* Initialize Leading Edge RTC device */
static void *
leading_edge_rtc_init(const device_t *info)
{
    leading_edge_rtc_t *dev;

    leading_edge_rtc_log("Leading Edge RTC: Initializing at I/O 0x300-0x31F\n");

    /* Allocate device structure */
    dev = (leading_edge_rtc_t *) calloc(1, sizeof(leading_edge_rtc_t));
    if (!dev)
        return NULL;

    /* Set up NVR backend */
    dev->nvr.data = dev;
    dev->nvr.size = MM67_REGS;
    dev->nvr.irq = -1; /* No IRQ by default (not typically used on Leading Edge) */
    dev->nvr.reset = mm67_reset;
    dev->nvr.start = mm67_start;
    dev->nvr.tick = mm67_tick;
    dev->nvr.fn = "leading_edge_rtc";
    
    /* Year stored in MM67_AL_DOM register (non-standard, base-80 BCD) */
    dev->year_reg = MM67_AL_DOM;

    /* Register I/O handlers for ports 0x300-0x31F (32 ports) */
    io_sethandler(0x0300, 32, mm67_read, NULL, NULL, mm67_write, NULL, NULL, dev);

    /* Initialize NVR backend */
    nvr_init(&dev->nvr);

    leading_edge_rtc_log("Leading Edge RTC: Initialization complete\n");

    return dev;
}

/* Close Leading Edge RTC device */
static void
leading_edge_rtc_close(void *priv)
{
    leading_edge_rtc_t *dev = (leading_edge_rtc_t *) priv;

    leading_edge_rtc_log("Leading Edge RTC: Closing\n");

    /* Remove I/O handlers */
    io_removehandler(0x0300, 32, mm67_read, NULL, NULL, mm67_write, NULL, NULL, dev);

    /* Free device structure */
    free(dev);
}

/* Leading Edge Model D RTC device definition */
const device_t leading_edge_rtc_device = {
    .name          = "Leading Edge Model D RTC",
    .internal_name = "leading_edge_rtc",
    .flags         = 0, /* No special flags - integrated device */
    .local         = 0,
    .init          = leading_edge_rtc_init,
    .close         = leading_edge_rtc_close,
    .reset         = NULL,
    .available     = NULL,
    .speed_changed = NULL,
    .force_redraw  = NULL,
    .config        = NULL
};
