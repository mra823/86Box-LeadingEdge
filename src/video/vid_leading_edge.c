/*
 * 86Box    A hypervisor and IBM PC system emulator that specializes in
 *          running old operating systems and software designed for IBM
 *          PC systems and compatibles from 1981 through fairly recent
 *          system designs based on the PCI bus.
 *
 *          This file is part of the 86Box distribution.
 *
 *          Emulation of the Leading Edge Model D integrated video adapter.
 *
 *          This adapter is CGA-compatible with an additional proprietary
 *          640×200×16 color mode accessed via register 0x3DF. The
 *          infrastructure for the extended mode is implemented, but the
 *          actual rendering is not due to lack of documentation and
 *          software that uses this mode.
 *
 *          The proprietary mode will fall back to standard CGA rendering
 *          until proper implementation details can be determined from
 *          testing with actual software or technical documentation.
 *
 * Authors: Matt Allen, <matt@mattallen.xyz>
 *
 *          Copyright 2025 Matt Allen.
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <wchar.h>
#include <86box/86box.h>
#include "cpu.h"
#include <86box/io.h>
#include <86box/timer.h>
#include <86box/pit.h>
#include <86box/mem.h>
#include <86box/rom.h>
#include <86box/device.h>
#include <86box/video.h>
#include <86box/vid_cga.h>
#include <86box/vid_cga_comp.h>
#include <86box/vid_leading_edge.h>
#include <86box/plat_unused.h>

/* Extended mode register (estimated address based on similar systems) */
#define LE_EXT_MODE_REGISTER 0x3DF

/* Extended mode flags */
#define LE_MODE_640x200x16 0x01  /* Enable proprietary 640×200×16 mode */
#define LE_MODE_ENABLE_EXT 0x80  /* Enable extended features */

/* CGA composite mode constants (from vid_cga.c) */
#define CGA_RGB       0
#define CGA_COMPOSITE 1

#define COMPOSITE_OLD 0
#define COMPOSITE_NEW 1

static video_timings_t timing_leading_edge = { 
    .type = VIDEO_ISA, 
    .write_b = 8, .write_w = 16, .write_l = 32, 
    .read_b = 8, .read_w = 16, .read_l = 32 
};

void
le_cga_recalctimings(le_cga_t *le_cga)
{
    double _dispontime;
    double _dispofftime;
    double disptime;

    if (le_cga->cga.cgamode & CGA_MODE_FLAG_HIGHRES) {
        disptime    = le_cga->cga.crtc[CGA_CRTC_HTOTAL] + 1;
        _dispontime = le_cga->cga.crtc[CGA_CRTC_HDISP];
    } else {
        disptime    = (le_cga->cga.crtc[CGA_CRTC_HTOTAL] + 1) << 1;
        _dispontime = le_cga->cga.crtc[CGA_CRTC_HDISP] << 1;
    }

    _dispofftime = disptime - _dispontime;
    _dispontime *= CGACONST / 2;
    _dispofftime *= CGACONST / 2;
    le_cga->cga.dispontime  = (uint64_t) (_dispontime);
    le_cga->cga.dispofftime = (uint64_t) (_dispofftime);
}

void
le_cga_out(uint16_t addr, uint8_t val, void *priv)
{
    le_cga_t *le_cga = (le_cga_t *) priv;

    switch (addr) {
        case 0x3d4:
        case 0x3d5:
        case 0x3d8:
        case 0x3d9:
            /* Standard CGA registers - pass through to CGA core */
            cga_out(addr, val, &le_cga->cga);
            break;

        case LE_EXT_MODE_REGISTER:
            /* Extended mode control register */
            le_cga->ext_mode = val;
            break;

        default:
            break;
    }
}

uint8_t
le_cga_in(uint16_t addr, void *priv)
{
    le_cga_t *le_cga = (le_cga_t *) priv;
    uint8_t ret = 0xff;

    switch (addr) {
        case 0x3d4:
        case 0x3d5:
        case 0x3da:
            /* Standard CGA registers - pass through to CGA core */
            ret = cga_in(addr, &le_cga->cga);
            break;

        case LE_EXT_MODE_REGISTER:
            /* Read back extended mode register */
            ret = le_cga->ext_mode;
            break;

        default:
            break;
    }

    return ret;
}

void
le_cga_write(uint32_t addr, uint8_t val, void *priv)
{
    le_cga_t *le_cga = (le_cga_t *) priv;

    /* Write to VRAM - handle both standard CGA and extended modes */
    le_cga->cga.vram[addr & 0x7FFF] = val;
    
    if (le_cga->cga.snow_enabled) {
        /* CGA snow effect simulation */
        int offset = ((timer_get_remaining_u64(&le_cga->cga.timer) / CGACONST) * 4) & 0xfc;
        le_cga->cga.charbuffer[offset]     = le_cga->cga.vram[addr & 0x7fff];
        le_cga->cga.charbuffer[offset | 1] = le_cga->cga.vram[addr & 0x7fff];
    }
}

uint8_t
le_cga_read(uint32_t addr, void *priv)
{
    le_cga_t *le_cga = (le_cga_t *) priv;

    if (le_cga->cga.snow_enabled) {
        /* CGA snow effect simulation */
        int offset = ((timer_get_remaining_u64(&le_cga->cga.timer) / CGACONST) * 4) & 0xfc;
        le_cga->cga.charbuffer[offset]     = le_cga->cga.vram[addr & 0x7fff];
        le_cga->cga.charbuffer[offset | 1] = le_cga->cga.vram[addr & 0x7fff];
    }

    return (le_cga->cga.vram[addr & 0x7FFF]);
}

static void
le_cga_render_640x200x16(le_cga_t *le_cga)
{
    /* 
     * Proprietary 640×200×16 mode rendering (NOT IMPLEMENTED)
     * 
     * The Leading Edge Model D supports a proprietary 640×200×16 color mode
     * that is not documented. The mode is enabled via extended mode register
     * at port 0x3DF, but the exact pixel packing format, memory layout, and
     * palette behavior are unknown.
     * 
     * No software has been found that uses this mode, and no technical
     * documentation exists. Until such software or documentation surfaces,
     * this mode falls back to standard CGA rendering.
     * 
     * If you have software that uses this mode or technical documentation,
     * please contribute to the 86Box project!
     * 
     * Infrastructure is in place for:
     * - Extended mode register at 0x3DF (LE_EXT_MODE_REGISTER)
     * - Mode detection flags (LE_MODE_640x200x16, LE_MODE_ENABLE_EXT)
     * - Memory handlers for extended VRAM access
     * 
     * What's needed:
     * - 4-bit per pixel framebuffer layout specification
     * - Palette mapping (16 colors from which registers?)
     * - Memory addressing scheme (planar, packed, interleaved?)
     * - Actual software to test against
     */
    
    /* Fall back to standard CGA rendering */
    cga_poll(&le_cga->cga);
}

void
le_cga_poll(void *priv)
{
    le_cga_t *le_cga = (le_cga_t *) priv;
    
    /* Check if extended 640×200×16 mode is enabled */
    if ((le_cga->ext_mode & LE_MODE_ENABLE_EXT) && 
        (le_cga->ext_mode & LE_MODE_640x200x16)) {
        /* Render proprietary mode */
        le_cga_render_640x200x16(le_cga);
    } else {
        /* Standard CGA rendering */
        cga_poll(&le_cga->cga);
    }
}

static void
le_cga_close(void *priv)
{
    le_cga_t *le_cga = (le_cga_t *) priv;

    free(le_cga->cga.vram);
    free(le_cga);
}

static void
le_cga_speed_changed(void *priv)
{
    le_cga_t *le_cga = (le_cga_t *) priv;

    cga_recalctimings(&le_cga->cga);
}

static void *
le_cga_init(const device_t *info)
{
    le_cga_t *le_cga = calloc(1, sizeof(le_cga_t));

    /* Inform video subsystem about device type */
    video_inform(VIDEO_FLAG_TYPE_CGA, &timing_leading_edge);
    
    /* Initialize CGA settings */
    le_cga->cga.composite = 0;  /* RGB mode */
    le_cga->cga.revision = COMPOSITE_OLD;  /* Composite revision */
    le_cga->cga.snow_enabled = 1;  /* Enable CGA snow effect */
    le_cga->cga.rgb_type = 0;  /* Standard RGB */
    le_cga->cga.double_type = 0;  /* No doubling */
    
    /* Allocate 32KB video RAM (standard CGA amount) */
    le_cga->cga.vram = malloc(0x8000);
    if (!le_cga->cga.vram) {
        free(le_cga);
        return NULL;
    }
    memset(le_cga->cga.vram, 0, 0x8000);
    
    /* Initialize extended mode register to disabled */
    le_cga->ext_mode = 0x00;

    /* Initialize CGA composite video system */
    cga_comp_init(le_cga->cga.revision);
    
    /* Set up timer for screen refresh */
    timer_add(&le_cga->cga.timer, le_cga_poll, le_cga, 1);

    /* Set up memory mapping for video RAM */
    mem_mapping_add(&le_cga->cga.mapping, 0xb8000, 0x8000,
                    le_cga_read, NULL, NULL,
                    le_cga_write, NULL, NULL,
                    NULL, MEM_MAPPING_EXTERNAL, le_cga);

    /* Register I/O handlers for CGA ports */
    io_sethandler(0x03d0, 0x0010,
                  le_cga_in, NULL, NULL,
                  le_cga_out, NULL, NULL, le_cga);

    /* Set overscan for proper display borders */
    overscan_x = overscan_y = 16;
    
    /* Initialize CGA palette */
    cga_palette = (le_cga->cga.rgb_type << 1);
    cgapal_rebuild();
    update_cga16_color(le_cga->cga.cgamode);
    
    /* Initialize interpolation for display scaling */
    cga_interpolate_init();
    
    /* Load IBM MDA font ROM (CGA uses the MDA font) */
    loadfont(FONT_IBM_MDA_437_PATH, 0);
    
    /* Set monitor composite mode flag */
    monitors[monitor_index_global].mon_composite = !!le_cga->cga.composite;

    return le_cga;
}

const device_t leading_edge_video_device = {
    .name          = "Leading Edge Model D Integrated Video",
    .internal_name = "leading_edge_video",
    .flags         = DEVICE_ISA,
    .local         = 0,
    .init          = le_cga_init,
    .close         = le_cga_close,
    .reset         = NULL,
    .available     = NULL,
    .speed_changed = le_cga_speed_changed,
    .force_redraw  = NULL,
    .config        = NULL
};
