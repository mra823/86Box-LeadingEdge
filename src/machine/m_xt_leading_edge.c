/*
 * 86Box    A hypervisor and IBM PC system emulator that specializes in
 *          running old operating systems and software designed for IBM
 *          PC systems and compatibles from 1981 through fairly recent
 *          system designs based on the PCI bus.
 *
 *          This file is part of the 86Box distribution.
 *
 *          Emulation of the Leading Edge Model D.
 *
 *          The Leading Edge Model D (1985) was an IBM PC/XT-compatible
 *          system featuring highly integrated design with on-board video,
 *          disk controllers, RTC, and I/O ports. It used a Phoenix BIOS
 *          and came in standard (4.77 MHz) and turbo (7.16 MHz) variants.
 *
 * Authors: Matt Allen, <matt@mattallen.xyz>
 *
 *          Copyright 2025 Matt Allen.
 */
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#define HAVE_STDARG_H
#include <86box/86box.h>
#include "cpu.h"
#include <86box/timer.h>
#include <86box/io.h>
#include <86box/device.h>
#include <86box/mem.h>
#include <86box/rom.h>
#include <86box/nmi.h>
#include <86box/pit.h>
#include <86box/pic.h>
#include <86box/ppi.h>
#include <86box/dma.h>
#include <86box/fdd.h>
#include <86box/fdc.h>
#include <86box/fdc_ext.h>
#include <86box/gameport.h>
#include <86box/keyboard.h>
#include <86box/machine.h>

/*
 * Leading Edge Model D machine initialization
 *
 * This function initializes the Leading Edge Model D, an XT-compatible
 * system with integrated peripherals. The machine features:
 * - Intel 8088 CPU at 4.77 MHz (standard) or 7.16 MHz (turbo)
 * - Phoenix BIOS v2.01
 * - Integrated CGA-compatible video with proprietary modes
 * - Integrated floppy disk controller
 * - Integrated serial and parallel ports
 * - Non-standard RTC at port 0x300
 * - 256-640 KB RAM (expandable to 768 KB on some models)
 */
int
machine_xt_leading_edge_d_init(const machine_t *model)
{
    int ret;

    /*
     * Load Phoenix BIOS v2.13 (16KB at 0xFC000)
     *
     * The Leading Edge Model D uses Phoenix BIOS v2.13 dated 12/27/85.
     * This includes the extended video BIOS.
     *
     * Note: The 23096000.BIN and 23097000.BIN files are option ROMs
     * and not needed for basic operation.
     */
    ret = bios_load_linear("roms/machines/leading_edge_d/Phoenix_BIOS_v2.13.bin",
                           0x000fc000, 16384, 0);

    /* Return early if BIOS-only mode or load failed */
    if (bios_only || !ret)
        return ret;

    /*
     * Add XT keyboard controller
     * The Leading Edge Model D uses a standard XT-style keyboard interface
     */
    device_add(&kbc_xt_device);

    /*
     * Initialize common hardware components
     *
     * This sets up the standard PC/XT chipset including:
     * - Intel 8237 DMA controller
     * - Intel 8253 Programmable Interval Timer (PIT)
     * - Intel 8259 Programmable Interrupt Controller (PIC)
     * - Intel 8255 Programmable Peripheral Interface (PPI)
     * - Memory configuration and mapping
     */
    machine_common_init(model);

    /*
     * Configure XT-specific timing
     *  
     * Set PIT channel 1 to use the XT-style refresh timer
     */
    pit_devs[0].set_out_func(pit_devs[0].data, 1, pit_refresh_timer_xt);

    /* Initialize NMI (Non-Maskable Interrupt) */
    nmi_init();

    /* Set standard gameport device */
    standalone_gameport_type = &gameport_200_device;

    /*
     * Configure PIT for XT-style DRAM refresh timing
     *
     * The Leading Edge Model D uses standard XT DRAM refresh timing.
     * Channel 1 of the PIT generates the DRAM refresh signal, which
     * triggers DMA channel 0 to refresh system memory.
     */
    pit_devs[0].set_out_func(pit_devs[0].data, 1, pit_refresh_timer_xt);

    /*
     * Add integrated floppy disk controller if configured as internal
     *
     * The Leading Edge Model D has an integrated FDC on the motherboard
     * that supports up to two 360KB 5.25" floppy drives. If the user
     * has configured the FDC as "Internal" in settings, we add the
     * standard XT FDC device.
     */
    if (fdc_current[0] == FDC_INTERNAL)
        device_add(&fdc_xt_device);

    /*
     * Initialize NMI (Non-Maskable Interrupt) handler
     *
     * The NMI handler manages parity errors and other critical
     * hardware conditions that require immediate attention.
     */
    nmi_init();

    /*
     * Configure game port for optional joystick support
     *
     * The Leading Edge Model D supports an optional game port
     * at the standard I/O address (0x200-0x20F).
     */
    standalone_gameport_type = &gameport_device;

    /*
     * Note: Leading Edge Model D has integrated CGA-compatible video
     *
     * The system includes integrated video that provides both MDA
     * (monochrome) and CGA (color) compatibility modes, plus a
     * proprietary 640×200×16 color mode. The standard CGA device
     * should be selected by the user in the video configuration.
     *
     * For full accuracy, a custom video device could be implemented
     * to support the proprietary mode, but standard CGA compatibility
     * is sufficient for most software.
     */

    /*
     * Note: Leading Edge Model D has RTC at non-standard port 0x300
     *
     * The system includes a battery-backed Real-Time Clock at I/O port
     * 0x300-0x31F, which is non-standard (IBM AT uses 0x70-0x7F).
     * This RTC requires a DOS driver (CLOCK.SYS or CLKDVR.SYS) to function.
     *
     * Important: This address conflicts with the default XT-IDE address.
     * Users should configure XT-IDE to use an alternate address if both
     * are needed.
     *
     * A custom RTC device implementation could be added in the future
     * for full hardware accuracy, but it is not essential for basic
     * operation since DOS will use the BIOS time functions.
     */

    return ret;
}

/*
 * Leading Edge Model D Turbo (7.16 MHz) variant initialization
 *
 * This function initializes the turbo variant of the Leading Edge Model D,
 * which runs at 7.16 MHz instead of the standard 4.77 MHz. These were
 * typically the "E" models (DC-2010E/DC-2011E).
 *
 * The hardware is identical except for the CPU speed, which is controlled
 * by the machine table configuration, so we simply call the standard init.
 */
int
machine_xt_leading_edge_d_turbo_init(const machine_t *model)
{
    /* For now, use same init - speed is controlled by CPU configuration */
    return machine_xt_leading_edge_d_init(model);
}
