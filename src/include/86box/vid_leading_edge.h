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
 * Authors: Matt Allen
 *
 *          Copyright 2025 Matt Allen.
 */
#ifndef VIDEO_LEADING_EDGE_H
#define VIDEO_LEADING_EDGE_H

#include <86box/vid_cga.h>

typedef struct le_cga_t {
    cga_t    cga;           /* Base CGA structure */
    uint8_t  ext_mode;      /* Extended mode control register (0x3DF) */
} le_cga_t;

void    le_cga_recalctimings(le_cga_t *le_cga);
void    le_cga_out(uint16_t addr, uint8_t val, void *priv);
uint8_t le_cga_in(uint16_t addr, void *priv);
void    le_cga_write(uint32_t addr, uint8_t val, void *priv);
uint8_t le_cga_read(uint32_t addr, void *priv);
void    le_cga_poll(void *priv);

#ifdef EMU_DEVICE_H
extern const device_t leading_edge_video_device;
#endif

#endif /* VIDEO_LEADING_EDGE_H */
