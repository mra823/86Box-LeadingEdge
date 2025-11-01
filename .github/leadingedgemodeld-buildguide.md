# Leading Edge Model D Emulation - 86Box Developer Build Guide

## Overview

This guide provides complete implementation instructions for adding Leading Edge Model D emulation support to the 86Box emulator. The Leading Edge Model D (1985) was an IBM PC/XT-compatible featuring highly integrated design with on-board video, disk controllers, RTC, and I/O ports.

**Target Platform**: 86Box v5.2+ (C codebase, CMake build system)  
**Machine Class**: XT-compatible (8088 CPU, 4.77/7.16 MHz)  
**Difficulty**: Intermediate (requires understanding of 86Box device system)

## Prerequisites

### Required Knowledge
- C programming language
- 86Box codebase structure and build system
- IBM PC/XT architecture and memory mapping
- Device driver concepts and I/O port handling

### Required Resources
- Leading Edge Model D BIOS ROM dump (Phoenix v2.01, 8KB)
- 86Box source code: https://github.com/86Box/86Box
- 86Box ROM repository: https://github.com/86Box/roms
- Technical documentation (available on Internet Archive)

### Development Environment
- CMake 3.15+
- C compiler (GCC, Clang, or MSVC)
- Git for version control

## Leading Edge Model D Hardware Specifications

### Core System Components

```
CPU:          Intel 8088 @ 4.77 MHz (standard) or 7.16 MHz (turbo on E models)
Coprocessor:  Intel 8087 socket provided (optional)
RAM:          256-640 KB standard, 768 KB on some revisions
BIOS:         Phoenix Technologies v2.01 (8KB at F000:0000)
Bus:          4× full-length 8-bit ISA expansion slots
Form Factor:  Proprietary compact desktop (254mm × 218mm motherboard)
```

### Integrated Components

**Chipset (Standard IBM PC/XT compatible):**
- Intel 8237A (DMA Controller)
- Intel 8253 (Programmable Interval Timer)
- Intel 8255A (Programmable Peripheral Interface)
- Intel 8259A (Programmable Interrupt Controller)
- Intel 8284A (Clock Generator)
- Intel 8288 (Bus Controller)

**Video Subsystem:**
- Motorola MC6845 CRT Controller
- MDA (Monochrome) and CGA (Color) compatible modes
- Proprietary 640×200×16 color mode (non-EGA compatible)
- Integrated video RAM

**Disk Controller:**
- Integrated floppy disk controller on motherboard
- Supports 2× 360KB 5.25" floppy drives
- Hard disk requires separate MFM controller card

**Real-Time Clock:**
- Battery-backed RTC at I/O port 300h (non-standard)
- Requires DOS driver (CLOCK.SYS/CLKDVR.SYS)
- **Important**: Conflicts with XT-IDE default address

**I/O Ports:**
- 1× RS-232 serial port (integrated)
- 1× parallel printer port (integrated)
- XT-style keyboard connector (5-pin DIN)

### Memory Map

```
0x00000-0x9FFFF   RAM (640KB conventional memory)
0xA0000-0xBFFFF   Video RAM (CGA/MDA framebuffer)
0xC0000-0xC7FFF   Expansion ROM (32KB)
0xC8000-0xC9FFF   Hard disk controller ROM (if present)
0xCA000-0xEFFFF   Expansion ROM space
0xF0000-0xFFFFF   System BIOS ROM (Phoenix, 64KB total, 8KB used)
```

### I/O Port Map

```
0x060-0x06F   Keyboard controller (8255 PPI)
0x200-0x20F   Game port (optional)
0x300-0x31F   RTC (Leading Edge specific - non-standard)
0x3B0-0x3BF   MDA video ports
0x3D0-0x3DF   CGA video ports
0x3F0-0x3F7   Floppy disk controller
0x3F8-0x3FF   Serial port (COM1)
0x378-0x37F   Parallel port (LPT1)
```

## 86Box Architecture Overview

### Machine Definition System

86Box uses a table-driven machine registration system where each machine is defined by a `machine_t` structure containing:
- Display name and internal identifier
- CPU configuration and supported speeds
- RAM limits and step increments
- Bus type flags (ISA, EISA, VLB, PCI)
- Initialization function pointer
- Optional device getter for integrated hardware

### Device Framework

All hardware components use the `device_t` structure:
- **Init callback**: Allocates state, registers I/O handlers, initializes hardware
- **Close callback**: Cleanup and deallocation
- **Reset callback**: Reset hardware state
- **Configuration**: User-facing settings
- **Availability check**: Validates ROM files and dependencies

### Initialization Flow

```
1. Machine selection by user (from machine table)
2. Configuration loaded from 86box.cfg
3. Machine init function called
4. BIOS ROM loaded into memory
5. Common initialization (machine_common_init)
6. Device registration (keyboard, FDC, etc.)
7. Hardware configuration (PIT, PIC, DMA, NMI)
8. Chipset-specific setup
9. System boot from BIOS
```

## Implementation Plan

### Phase 1: File Structure Setup

**Step 1.1**: Create machine implementation file
- Location: `src/machine/m_xt_leading_edge.c`
- Purpose: Machine-specific initialization and configuration

**Step 1.2**: Prepare ROM directory
- Location: `roms/machines/leading_edge_d/`
- Contents: Phoenix BIOS ROM dump(s)

### Phase 2: Basic Machine Implementation

**Step 2.1**: Implement machine initialization function
**Step 2.2**: Add machine table entry
**Step 2.3**: Update machine.h header with function prototype

### Phase 3: Integrated Components

**Step 3.1**: Implement integrated video (CGA/MDA compatibility)
**Step 3.2**: Configure integrated FDC
**Step 3.3**: Implement RTC at port 300h (optional, for accuracy)
**Step 3.4**: Configure serial/parallel ports

### Phase 4: Testing and Refinement

**Step 4.1**: BIOS POST verification
**Step 4.2**: Operating system installation tests
**Step 4.3**: Hardware compatibility validation
**Step 4.4**: Documentation and cleanup

## File Structure and Modifications

### Files to Create

#### 1. `src/machine/m_xt_leading_edge.c`
Main machine implementation file containing:
- Machine initialization function
- Device configuration
- BIOS loading
- Hardware setup

#### 2. `roms/machines/leading_edge_d/bios.bin`
Phoenix BIOS v2.01 ROM dump (8KB)

### Files to Modify

#### 1. `src/include/86box/machine.h`
Add function prototype:
```c
/* Leading Edge */
extern int machine_xt_leading_edge_d_init(const machine_t *model);
```

#### 2. `src/machine/machine_table.c`
Add machine table entry in the XT section

#### 3. `src/machine/CMakeLists.txt` (if needed)
Ensure `m_xt_leading_edge.c` is included in build sources

## Code Implementation

### Step 1: Machine Implementation File

Create `src/machine/m_xt_leading_edge.c`:

```c
/*
 * 86Box    A hypervisor and IBM PC system emulator that specializes in
 *          running old operating systems and software designed for IBM
 *          PC systems and compatibles from 1981 through fairly recent
 *          system designs based on the PCI bus.
 *
 *          Emulation of the Leading Edge Model D.
 *
 * Authors: [Your Name]
 *
 *          Copyright 2025 [Your Name]
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
#include <86box/lpt.h>
#include <86box/serial.h>
#include <86box/video.h>
#include <86box/vid_cga.h>
#include <86box/machine.h>

/* Leading Edge Model D initialization */
int
machine_xt_leading_edge_d_init(const machine_t *model)
{
    int ret;

    /* Load Phoenix BIOS v2.01 (8KB at FE000h) */
    ret = bios_load_linear("roms/machines/leading_edge_d/bios.bin",
                           0x000fe000, 8192, 0);

    /* Return early if BIOS-only mode or load failed */
    if (bios_only || !ret)
        return ret;

    /* Initialize common XT hardware components */
    machine_common_init(model);

    /* Configure PIT for XT-style DRAM refresh timing */
    pit_devs[0].set_out_func(pit_devs[0].data, 1, pit_refresh_timer_xt);

    /* Add standard XT keyboard controller */
    device_add(&keyboard_xt_device);

    /* Add integrated floppy disk controller if configured as internal */
    if (fdc_current[0] == FDC_INTERNAL)
        device_add(&fdc_xt_device);

    /* Initialize NMI (Non-Maskable Interrupt) handler */
    nmi_init();

    /* Configure game port */
    standalone_gameport_type = &gameport_device;

    /* Configure integrated parallel port at standard address */
    lpt1_remove();
    lpt1_init(0x378);

    /* Configure integrated serial port (COM1 at 3F8h, IRQ 4) */
    serial_setup(serial_attach(0, NULL, NULL), SERIAL1_ADDR, SERIAL1_IRQ);

    /* 
     * Note: Leading Edge Model D has integrated CGA-compatible video.
     * In 86Box, this is typically handled through user selection of
     * "Internal device" in video configuration. For machines where
     * video is truly fixed, use MACHINE_VIDEO and MACHINE_VIDEO_FIXED flags.
     * 
     * For now, we rely on user configuration to select CGA as video card.
     * Advanced implementation could add custom video device for the
     * proprietary 640x200x16 mode.
     */

    /*
     * Note: Leading Edge Model D has RTC at non-standard port 300h.
     * This conflicts with XT-IDE default address. Implementing this
     * requires custom device code. For initial implementation, we
     * omit this feature as it's not essential for basic compatibility.
     * 
     * Future enhancement:
     * device_add(&leading_edge_rtc_device);
     */

    return ret;
}

/* 
 * Leading Edge Model D Turbo (7.16 MHz) variant 
 * This would be for the "E" models (DC-2010E/DC-2011E)
 */
int
machine_xt_leading_edge_d_turbo_init(const machine_t *model)
{
    /* For now, use same init - speed is controlled by CPU configuration */
    return machine_xt_leading_edge_d_init(model);
}
```

### Step 2: Machine Table Entry

Add to `src/machine/machine_table.c` in the appropriate XT section:

```c
/* Leading Edge Model D */
{
    .name          = "[8088] Leading Edge Model D",
    .internal_name = "leading_edge_d",
    .type          = MACHINE_TYPE_8088,
    .chipset       = MACHINE_CHIPSET_DISCRETE,
    .init          = machine_xt_leading_edge_d_init,
    .p1_handler    = NULL,
    .gpio_handler  = NULL,
    .available_flag= 0,
    .gpio_acpi_handler = NULL,
    .cpu           = {
        .package   = CPU_PKG_8088,
        .block     = CPU_BLOCK_NONE,
        .min_bus   = 4772728,    /* 4.77 MHz */
        .max_bus   = 4772728,
        .min_voltage = 0,
        .max_voltage = 0,
        .min_multi = 0,
        .max_multi = 0
    },
    .bus_flags     = MACHINE_ISA,
    .flags         = MACHINE_PC | MACHINE_VIDEO,  /* MACHINE_VIDEO indicates integrated video */
    .ram           = {
        .min       = 256,        /* 256 KB minimum */
        .max       = 640,        /* 640 KB maximum */
        .step      = 64
    },
    .nvrmask       = 0,          /* No NVRAM on XT systems */
    .kbc_device    = NULL,
    .kbc_p1        = 0xff,
    .gpio          = 0xffffffff,
    .gpio_acpi     = 0xffffffff,
    .device        = NULL,
    .fdc_device    = NULL,
    .sio_device    = NULL,
    .vid_device    = NULL,       /* Could point to custom CGA device */
    .snd_device    = NULL,
    .net_device    = NULL
},

/* Leading Edge Model D Turbo (E models - 7.16 MHz) */
{
    .name          = "[8088] Leading Edge Model D Turbo",
    .internal_name = "leading_edge_d_turbo",
    .type          = MACHINE_TYPE_8088,
    .chipset       = MACHINE_CHIPSET_DISCRETE,
    .init          = machine_xt_leading_edge_d_turbo_init,
    .p1_handler    = NULL,
    .gpio_handler  = NULL,
    .available_flag= 0,
    .gpio_acpi_handler = NULL,
    .cpu           = {
        .package   = CPU_PKG_8088,
        .block     = CPU_BLOCK_NONE,
        .min_bus   = 7159092,    /* 7.16 MHz */
        .max_bus   = 7159092,
        .min_voltage = 0,
        .max_voltage = 0,
        .min_multi = 0,
        .max_multi = 0
    },
    .bus_flags     = MACHINE_ISA,
    .flags         = MACHINE_PC | MACHINE_VIDEO,
    .ram           = {
        .min       = 512,        /* Later models came with 512 KB */
        .max       = 768,        /* Some turbo models supported 768 KB */
        .step      = 64
    },
    .nvrmask       = 0,
    .kbc_device    = NULL,
    .kbc_p1        = 0xff,
    .gpio          = 0xffffffff,
    .gpio_acpi     = 0xffffffff,
    .device        = NULL,
    .fdc_device    = NULL,
    .sio_device    = NULL,
    .vid_device    = NULL,
    .snd_device    = NULL,
    .net_device    = NULL
},
```

### Step 3: Header File Update

Add to `src/include/86box/machine.h` in the appropriate section:

```c
/* machine/m_xt_leading_edge.c */
extern int machine_xt_leading_edge_d_init(const machine_t *model);
extern int machine_xt_leading_edge_d_turbo_init(const machine_t *model);
```

### Step 4: Advanced - Custom RTC Device (Optional)

If implementing the non-standard RTC at port 300h, create a custom device:

```c
/* Leading Edge Model D RTC device */
typedef struct {
    uint8_t regs[16];
    uint8_t addr_latch;
    int64_t base_time;
} leading_edge_rtc_t;

static void
le_rtc_write(uint16_t port, uint8_t val, void *priv)
{
    leading_edge_rtc_t *rtc = (leading_edge_rtc_t *)priv;
    
    switch (port & 0x1f) {
        case 0x00:  /* Address latch */
            rtc->addr_latch = val & 0x0f;
            break;
            
        case 0x01:  /* Data register */
            if (rtc->addr_latch < 16)
                rtc->regs[rtc->addr_latch] = val;
            break;
    }
}

static uint8_t
le_rtc_read(uint16_t port, void *priv)
{
    leading_edge_rtc_t *rtc = (leading_edge_rtc_t *)priv;
    uint8_t ret = 0xff;
    
    switch (port & 0x1f) {
        case 0x01:  /* Data register */
            if (rtc->addr_latch < 16)
                ret = rtc->regs[rtc->addr_latch];
            break;
    }
    
    return ret;
}

static void *
le_rtc_init(const device_t *info)
{
    leading_edge_rtc_t *rtc;
    
    rtc = (leading_edge_rtc_t *)malloc(sizeof(leading_edge_rtc_t));
    memset(rtc, 0, sizeof(leading_edge_rtc_t));
    
    /* Register I/O handlers at port 300h */
    io_sethandler(0x0300, 0x0020,
                  le_rtc_read, NULL, NULL,
                  le_rtc_write, NULL, NULL,
                  rtc);
    
    return rtc;
}

static void
le_rtc_close(void *priv)
{
    leading_edge_rtc_t *rtc = (leading_edge_rtc_t *)priv;
    
    io_removehandler(0x0300, 0x0020,
                     le_rtc_read, NULL, NULL,
                     le_rtc_write, NULL, NULL,
                     rtc);
    
    free(rtc);
}

const device_t leading_edge_rtc_device = {
    .name          = "Leading Edge Model D RTC",
    .internal_name = "leading_edge_rtc",
    .flags         = 0,
    .local         = 0,
    .init          = le_rtc_init,
    .close         = le_rtc_close,
    .reset         = NULL,
    .available     = NULL,
    .speed_changed = NULL,
    .force_redraw  = NULL,
    .config        = NULL
};
```

### Step 5: Advanced - Proprietary Video Mode (Optional)

The Leading Edge Model D has a proprietary 640×200×16 color mode. This requires custom video device implementation:

```c
/* 
 * Leading Edge Model D Enhanced CGA
 * Implements standard CGA plus proprietary 640x200x16 mode
 */
typedef struct {
    cga_t cga;              /* Base CGA structure */
    uint8_t ext_mode;       /* Extended mode register */
    uint8_t ext_palette[16];/* Extended palette registers */
} le_cga_t;

static void
le_cga_write(uint16_t addr, uint8_t val, void *priv)
{
    le_cga_t *le_cga = (le_cga_t *)priv;
    
    /* Handle extended registers at 3DFh */
    if (addr == 0x3df) {
        le_cga->ext_mode = val;
        /* Recalculate timings for extended mode */
        if (val & 0x80) {
            /* Enable 640x200x16 mode */
            le_cga->cga.cgamode = 0x1a;  /* High-res graphics */
            /* Additional mode setup... */
        }
        return;
    }
    
    /* Pass through to standard CGA handler */
    cga_write(addr, val, &le_cga->cga);
}

/* Similar pattern for le_cga_read, le_cga_init, etc. */
```

## BIOS ROM Requirements

### ROM File Specifications

**Filename**: `bios.bin` (or descriptive name like `phoenix_v201.bin`)  
**Size**: 8,192 bytes (8 KB)  
**Load Address**: 0xFE000 (segment F000:E000)  
**Format**: Raw binary dump  
**Checksum**: Verify with ROM checksum tools before use

### Obtaining BIOS ROM

**Legal sources:**
1. Dump from actual Leading Edge Model D hardware using ROM reader
2. Extract from Internet Archive software collections (verify licensing)
3. Use existing dumps from retro computing communities (check copyright status)

**Dumping methods:**
- Hardware EPROM programmer (Willem, TL866)
- Software dump using DEBUG.COM on real hardware
- Extract from BIOS update utilities

### ROM Directory Structure

```
86Box/roms/machines/leading_edge_d/
├── bios.bin                    # Phoenix v2.01 (primary)
├── phoenix_v201_rev5.bin       # Alternative BIOS version (optional)
└── README.txt                  # ROM documentation
```

### BIOS Validation

Before using ROM in emulator:
1. Verify file size is exactly 8192 bytes
2. Check for valid x86 code at entry points
3. Validate BIOS signature strings (e.g., "Phoenix")
4. Test ROM loads without crashes in 86Box

### ROM Repository Submission

When contributing ROMs to 86Box/roms repository:

1. **Prepare descriptive commit**:
```bash
git add roms/machines/leading_edge_d/
git commit -m "Add Leading Edge Model D Phoenix BIOS v2.01"
```

2. **Include metadata in commit message**:
```
Add Leading Edge Model D Phoenix BIOS v2.01

- Model: Leading Edge Model D (DC-2010/DC-2011)
- BIOS: Phoenix Technologies v2.01
- Size: 8KB
- Date: Circa 1985
- Source: Dumped from actual hardware
- Verified: Boots DOS 3.3, passes diagnostics
```

3. **Submit PR after code PR is approved**:
- Code changes must be merged to 86Box master first
- ROM PR references the merged code PR number
- ROM PR will not merge until code is in master branch

## Testing Guidelines

### Phase 1: Basic Functionality

**Test 1.1: BIOS POST**
```
Expected: BIOS loads, performs POST, shows Phoenix copyright
Success criteria:
- No ROM load errors in log
- BIOS memory test completes
- System halts at "No boot device" or attempts boot
```

**Test 1.2: RAM Configuration**
```
Test configurations: 256KB, 512KB, 640KB, 768KB (if supported)
Expected: BIOS detects correct amount
Success criteria:
- Memory test shows correct size
- DOS reports correct conventional memory
```

**Test 1.3: CPU Speed**
```
Test standard 4.77 MHz and turbo 7.16 MHz variants
Expected: Correct CPU speed reported in BIOS
Success criteria:
- Speed matches machine definition
- Benchmarks show expected performance delta
```

### Phase 2: Operating System Installation

**Test 2.1: DOS Installation**
```
Test with: MS-DOS 3.3, 5.0, 6.22
Expected: Clean installation and boot
Success criteria:
- FORMAT.COM works correctly
- File operations succeed
- System boots from floppy and hard disk
```

**Test 2.2: Windows Installation**
```
Test with: Windows 3.0 (Real Mode)
Expected: Installation completes, Windows runs
Success criteria:
- Setup completes without errors
- Windows runs in Real Mode
- Applications launch successfully
```

### Phase 3: Hardware Compatibility

**Test 3.1: Floppy Disk Controller**
```
Expected: Read/write to floppy images
Success criteria:
- FORMAT /S creates bootable disks
- XCOPY transfers files correctly
- Multiple drives accessible
```

**Test 3.2: Video Output**
```
Test modes: MDA text, CGA text, CGA graphics
Expected: All standard modes work correctly
Success criteria:
- Text displays correctly in 80x25
- CGA 320x200x4 graphics render
- CGA 640x200x2 graphics render
- Mode switching works properly
```

**Test 3.3: Serial/Parallel Ports**
```
Expected: Ports detected and functional
Success criteria:
- MODE COM1 commands work
- Parallel port prints (if configured)
- Port detection in diagnostics
```

### Phase 4: Emulation Accuracy

**Test 4.1: Timing Accuracy**
```
Run timing-sensitive software (games, demos)
Expected: Correct speed, no glitches
Success criteria:
- Software runs at expected speed
- No timing-related hangs or crashes
```

**Test 4.2: Compatibility Testing**
```
Test with known XT software:
- Lotus 1-2-3
- WordPerfect 4.2
- dBASE III
- Popular games (Alley Cat, Flight Simulator 1.0)

Expected: Software runs as on real hardware
Success criteria:
- No unusual errors or crashes
- Keyboard input works correctly
- Save/load functions work
```

### Phase 5: Edge Cases

**Test 5.1: BIOS Setup**
```
Expected: BIOS setup accessible (if implemented)
Success criteria:
- Setup key (Del/F1) detected
- Settings can be changed
- Settings persist across reboots (if NVRAM implemented)
```

**Test 5.2: Hardware Conflicts**
```
Test with expansion cards that might conflict:
- XT-IDE at various addresses
- Additional video cards
- Sound cards

Expected: Graceful handling or documented conflicts
Success criteria:
- Document any incompatibilities
- Suggest workarounds
```

### Logging and Debugging

Enable detailed logging for troubleshooting:

```c
/* Add debug logging in machine init */
#ifdef ENABLE_LEADING_EDGE_LOG
int leading_edge_do_log = ENABLE_LEADING_EDGE_LOG;

static void
leading_edge_log(const char *fmt, ...)
{
    va_list ap;
    
    if (leading_edge_do_log) {
        va_start(ap, fmt);
        pclog_ex(fmt, ap);
        va_end(ap);
    }
}
#else
#define leading_edge_log(fmt, ...)
#endif

/* Use in code */
leading_edge_log("Leading Edge Model D: Initializing at %08X\n", address);
```

### Regression Testing

Before submitting PR:
1. Verify existing XT machines still work (IBM PC, IBM XT, Compaq Deskpro)
2. Run 86Box test suite (if available)
3. Test on multiple host platforms (Windows, Linux, macOS)
4. Verify no memory leaks with valgrind (Linux) or similar tools

## Integration Points Reference

### Key 86Box API Functions

#### BIOS Loading
```c
/* Load linear BIOS ROM */
int bios_load_linear(const char *fn, uint32_t addr, int sz, int off);

/* Load interleaved BIOS (even/odd split) */
int bios_load_interleaved(const char *fn1, const char *fn2, 
                          uint32_t addr, int sz, int off);
```

#### Machine Initialization
```c
/* Initialize common XT hardware */
void machine_common_init(const machine_t *model);

/* Initialize AT-specific hardware (for reference) */
void machine_at_common_init(const machine_t *model);
void machine_at_common_init_ex(const machine_t *model, int type);
```

#### Device Management
```c
/* Add device to system */
void device_add(const device_t *device);

/* Get device configuration value */
int device_get_config_int(const char *name);
```

#### Memory Management
```c
/* Set memory region attributes */
void mem_set_mem_state(uint32_t addr, uint32_t size, int state);

/* Memory state flags */
#define MEM_READ_INTERNAL    0x01
#define MEM_WRITE_INTERNAL   0x02
#define MEM_READ_EXTANY      0x10
#define MEM_WRITE_EXTANY     0x20
```

#### I/O Port Registration
```c
/* Register I/O port handlers */
void io_sethandler(uint16_t base, int size,
                   uint8_t (*inb)(uint16_t addr, void *priv),
                   uint16_t (*inw)(uint16_t addr, void *priv),
                   uint32_t (*inl)(uint16_t addr, void *priv),
                   void (*outb)(uint16_t addr, uint8_t val, void *priv),
                   void (*outw)(uint16_t addr, uint16_t val, void *priv),
                   void (*outl)(uint16_t addr, uint32_t val, void *priv),
                   void *priv);

/* Remove I/O port handlers */
void io_removehandler(uint16_t base, int size, ...);
```

#### Serial Port Configuration
```c
/* Attach serial device */
serial_t *serial_attach(int port, void *handler, void *priv);

/* Configure serial port */
void serial_setup(serial_t *dev, uint16_t addr, int irq);
```

#### Parallel Port Configuration
```c
/* Initialize parallel port */
void lpt1_init(uint16_t addr);

/* Remove parallel port */
void lpt1_remove(void);

/* Setup at specific address */
void lpt1_setup(uint16_t addr);
```

#### Timer and Interrupt Configuration
```c
/* Configure PIT output function */
pit_devs[0].set_out_func(pit_devs[0].data, int channel, 
                         void (*func)(int new_out, int old_out));

/* Standard timer functions */
void pit_refresh_timer_xt(int new_out, int old_out);  /* XT refresh */
void pit_refresh_timer_at(int new_out, int old_out);  /* AT refresh */
```

#### NMI Initialization
```c
/* Initialize NMI handler */
void nmi_init(void);
```

### Common Device Structures

```c
/* Standard XT keyboard */
extern const device_t keyboard_xt_device;

/* XT floppy controller */
extern const device_t fdc_xt_device;

/* Game port */
extern const device_t gameport_device;

/* Standard CGA */
extern const device_t cga_device;
```

### Machine Flags Reference

```c
/* Bus type flags */
#define MACHINE_ISA          0x00000001  /* ISA bus */
#define MACHINE_EISA         0x00000002  /* EISA bus */
#define MACHINE_VLB          0x00000004  /* VESA Local Bus */
#define MACHINE_PCI          0x00000008  /* PCI bus */
#define MACHINE_PS2          0x00000040  /* PS/2 architecture */

/* Machine feature flags */
#define MACHINE_PC           0x00000100  /* PC-class machine */
#define MACHINE_AT           0x00000200  /* AT-class machine */
#define MACHINE_IDE          0x00001000  /* Built-in IDE */
#define MACHINE_IDE_DUAL     0x00002000  /* Dual IDE channels */
#define MACHINE_VIDEO        0x00004000  /* Integrated video */
#define MACHINE_VIDEO_FIXED  0x00008000  /* Non-removable video */
#define MACHINE_APM          0x00040000  /* APM support */
#define MACHINE_ACPI         0x00080000  /* ACPI support */
```

## Build and Test Workflow

### 1. Prepare Development Environment

```bash
# Clone 86Box repository
git clone https://github.com/86Box/86Box.git
cd 86Box

# Create feature branch
git checkout -b feature/leading-edge-model-d

# Clone ROM repository separately
cd ..
git clone https://github.com/86Box/roms.git
```

### 2. Implement Changes

```bash
# Create machine implementation file
touch src/machine/m_xt_leading_edge.c

# Edit files (as documented above):
# - src/machine/m_xt_leading_edge.c
# - src/include/86box/machine.h
# - src/machine/machine_table.c

# Add BIOS ROM to local test directory
mkdir -p roms/machines/leading_edge_d/
cp /path/to/bios.bin roms/machines/leading_edge_d/
```

### 3. Build 86Box

```bash
# Configure with CMake
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Build
cmake --build .

# Run
./86Box
```

### 4. Test Implementation

```bash
# Launch 86Box with Leading Edge Model D selected
# Test all functionality as per testing guidelines above

# Check logs for errors
tail -f ~/.86Box/86box.log  # Linux
# or check logs in %USERPROFILE%\.86Box\ on Windows
```

### 5. Submit Pull Request

```bash
# Commit changes with clear message
git add src/machine/m_xt_leading_edge.c
git add src/include/86box/machine.h
git add src/machine/machine_table.c
git commit -m "Add Leading Edge Model D machine emulation

- Implement machine initialization for Model D and Model D Turbo
- Add Phoenix BIOS v2.01 support
- Configure integrated CGA video, FDC, serial/parallel ports
- Support both 4.77 MHz and 7.16 MHz variants
- Tested with DOS 3.3, 6.22 and Windows 3.0"

# Push to your fork
git push origin feature/leading-edge-model-d

# Create PR on GitHub with detailed description
```

### 6. Submit ROM PR (After Code Merge)

```bash
cd ../roms
git checkout -b feature/leading-edge-model-d-roms

# Add ROM files
mkdir -p machines/leading_edge_d
cp /path/to/bios.bin machines/leading_edge_d/

# Create README
cat > machines/leading_edge_d/README.txt << EOF
Leading Edge Model D BIOS ROMs

bios.bin - Phoenix Technologies v2.01 (8KB)
  - Original BIOS for DC-2010/DC-2011 models
  - Date: Circa 1985
  - Verified working with 86Box
EOF

# Commit
git add machines/leading_edge_d/
git commit -m "Add Leading Edge Model D Phoenix BIOS v2.01

- Phoenix BIOS v2.01 (8KB)
- Dumped from authentic hardware
- Verified boots DOS and Windows
- Relates to PR #XXXX in main repository"

# Push and create PR
git push origin feature/leading-edge-model-d-roms
```

## Advanced Enhancements

### Multiple BIOS Versions

To support multiple BIOS revisions:

```c
/* In machine table entry */
static const device_config_t leading_edge_bios_config[] = {
    {
        .name = "bios",
        .description = "BIOS Version",
        .type = CONFIG_SELECTION,
        .default_int = 0,
        .selection = {
            { .description = "Phoenix v2.01",     .value = 0 },
            { .description = "Phoenix v2.52",     .value = 1 },
            { .description = "" }
        }
    },
    { .name = "", .description = "", .type = CONFIG_END }
};

/* In init function */
int bios_ver = device_get_config_int("bios");
const char *bios_files[] = {
    "roms/machines/leading_edge_d/phoenix_v201.bin",
    "roms/machines/leading_edge_d/phoenix_v252.bin"
};
ret = bios_load_linear(bios_files[bios_ver], 0x000fe000, 8192, 0);
```

### Motherboard Revision Support

Support different motherboard revisions with varying capabilities:

```c
/* Different revisions have different max RAM */
static const device_config_t le_revision_config[] = {
    {
        .name = "revision",
        .description = "Motherboard Revision",
        .type = CONFIG_SELECTION,
        .default_int = 0,
        .selection = {
            { .description = "Rev 1 (640KB max)",  .value = 0 },
            { .description = "Rev 7 (768KB max)",  .value = 1 },
            { .description = "" }
        }
    },
    { .name = "", .description = "", .type = CONFIG_END }
};
```

### Proprietary Video Mode Implementation

Full implementation of 640×200×16 mode requires:
1. Custom video device structure extending CGA
2. Additional palette registers
3. Extended mode timing calculations
4. Proper framebuffer handling for 4-bit pixels

This is advanced and can be deferred to future enhancement after basic compatibility is established.

## Common Issues and Solutions

### Issue 1: BIOS Won't Load
**Symptoms**: Error message "Could not load BIOS", system won't boot  
**Solutions**:
- Verify ROM file exists at correct path
- Check ROM file size is exactly 8192 bytes
- Ensure ROM file has read permissions
- Verify load address (0xFE000 for XT BIOS)

### Issue 2: System Hangs After POST
**Symptoms**: BIOS completes but system freezes  
**Solutions**:
- Check PIT configuration (ensure XT refresh timer)
- Verify NMI initialization
- Check keyboard device is added correctly
- Ensure FDC device added if configured as internal

### Issue 3: No Video Output
**Symptoms**: Black screen, no display  
**Solutions**:
- Verify user selected CGA or MDA video card
- Check MACHINE_VIDEO flag set correctly
- Ensure machine allows video card selection
- Test with known-working video card (CGA)

### Issue 4: RAM Not Detected Correctly
**Symptoms**: Wrong amount of RAM shown in BIOS  
**Solutions**:
- Verify RAM limits in machine table entry
- Check RAM step value (typically 64KB for XT)
- Ensure machine_common_init() called
- Verify no memory mapping conflicts

### Issue 5: Keyboard Not Working
**Symptoms**: No keyboard input, can't type  
**Solutions**:
- Verify keyboard_xt_device added
- Check keyboard device added after machine_common_init()
- Ensure PPI (8255) initialized correctly
- Test with different keyboard emulation modes

## Performance Considerations

### Emulation Overhead

The Leading Edge Model D is an XT-class machine and should have minimal emulation overhead:
- Simple discrete chipset (no complex integrated chipsets)
- Standard 8088 emulation (well-optimized in 86Box)
- CGA video (simple framebuffer, low overhead)
- No DMA-heavy operations

### Optimization Tips

1. **ROM Caching**: ROMs are loaded once, cached in memory
2. **Video Updates**: Use dirty rectangle tracking for CGA updates
3. **Device Polling**: Minimize unnecessary device polling
4. **Timer Accuracy**: Balance accuracy vs. performance for PIT

### Expected Performance

On modern hardware (2020+):
- Should achieve full speed (4.77 MHz) with <5% CPU usage
- Turbo mode (7.16 MHz) should run easily at full speed
- No throttling needed for this simple architecture

## Documentation and Comments

### Code Documentation Standards

Use clear, descriptive comments:

```c
/*
 * Initialize Leading Edge Model D machine
 * 
 * This function sets up an XT-compatible system with:
 * - Phoenix BIOS v2.01 loaded at FE000h
 * - Integrated CGA-compatible video (user-selectable)
 * - Integrated floppy controller
 * - Integrated serial and parallel ports
 * - Standard XT keyboard controller
 * 
 * Notable differences from IBM PC/XT:
 * - RTC at port 300h (conflicts with XT-IDE)
 * - Proprietary 640x200x16 graphics mode (not implemented)
 * - Compact motherboard (non-standard form factor)
 * 
 * @param model Pointer to machine structure
 * @return 1 on success, 0 on failure (ROM load error)
 */
```

### Commit Message Format

Follow 86Box conventions:

```
Short summary (50 chars max)

Longer description explaining what changed and why.
Wrap at 72 characters.

- Bullet points for specific changes
- Reference issues/PRs as needed

Fixes #1234
```

## References and Resources

### Official Documentation
- 86Box Documentation: https://86box.readthedocs.io
- Device API: https://86box.readthedocs.io/en/latest/dev/api/device.html
- Build Guide: https://86box.readthedocs.io/en/latest/dev/buildguide.html

### Leading Edge Model D Resources
- Internet Archive: Search "Leading Edge Model D" for manuals
- The Retro Web: Motherboard specifications and jumper settings
- VCF Forum: Vintage Computer Federation discussions

### 86Box Development
- GitHub Repository: https://github.com/86Box/86Box
- ROM Repository: https://github.com/86Box/roms
- Discord Server: Active developer community
- IRC: #86Box on Libera.Chat

### IBM PC Architecture
- IBM PC/XT Technical Reference Manual
- Phoenix BIOS documentation
- "The Indispensable PC Hardware Book" by Hans-Peter Messmer
- "IBM PC Architecture" reference materials

## Summary Checklist

Before submitting your implementation:

- [ ] Machine implementation file created (`m_xt_leading_edge.c`)
- [ ] Machine table entry added with correct specifications
- [ ] Header file updated with function prototype
- [ ] BIOS ROM obtained and validated (8KB Phoenix v2.01)
- [ ] ROM directory created and ROM file added
- [ ] Code compiles without errors or warnings
- [ ] Basic POST completes successfully
- [ ] DOS boots and runs correctly
- [ ] Floppy disk controller functional
- [ ] Keyboard input works
- [ ] Video output correct (CGA text/graphics)
- [ ] Serial/parallel ports configured
- [ ] Memory detection correct (256-640KB)
- [ ] Documentation and comments added
- [ ] Code follows 86Box style guidelines
- [ ] Tested on multiple host platforms (if possible)
- [ ] Pull request prepared with clear description
- [ ] ROM PR ready (to submit after code merge)

## Conclusion

This guide provides complete implementation instructions for adding Leading Edge Model D emulation to 86Box. The implementation follows established patterns for XT-class machines while accommodating the Model D's integrated design.

Key aspects of this implementation:
- **Simple and maintainable**: Uses standard 86Box patterns
- **Well-documented**: Clear comments and commit messages
- **Tested thoroughly**: Comprehensive testing guidelines
- **Historically accurate**: Based on actual hardware specifications
- **Extensible**: Foundation for future enhancements (RTC, proprietary video)

The Leading Edge Model D represents an important milestone in PC clone history, and its emulation in 86Box preserves this legacy for future generations of retro computing enthusiasts.

For questions or assistance, consult the 86Box Discord server or GitHub discussions where the active developer community can provide guidance.

**Good luck with your implementation!**