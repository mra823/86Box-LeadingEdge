# 86Box AI Coding Agent Instructions

## Project Overview
86Box is a low-level x86 PC emulator (1981-2000s era) written in C (C11) and C++ (C++14). It emulates complete PC systems including CPUs (8086 to Pentium III), chipsets, video cards, sound cards, storage controllers, and peripherals at the hardware register level.

## Architecture & Core Concepts

### Device System (Central Pattern)
The emulator uses a unified device framework defined in `src/include/86box/device.h`:
- All emulated hardware implements `device_t` struct with lifecycle callbacks: `init`, `reset`, `close`, `speed_changed`
- Devices are registered via `device_add()` / `device_add_inst()` variants
- Each device has configuration via `device_config_t` array defining UI-exposed parameters (INTs, strings, selections, spinners)
- Device flags determine compatibility: `DEVICE_ISA`, `DEVICE_PCI`, `DEVICE_VLB`, `DEVICE_PS2_KBC`, etc.

**Example pattern** (from `src/chipset/intel_piix.c`):
```c
const device_t piix_device = {
    .name = "Intel PIIX",
    .internal_name = "piix",
    .flags = DEVICE_PCI,
    .init = piix_init,
    .close = piix_close,
    .reset = piix_reset,
    // ...
};
```

### Machine System
Machines are complete system configurations in `src/machine/`:
- `machine_t` structs define CPU, RAM, buses, and onboard devices (see `src/machine/machine_table.c` - 20K+ lines)
- Machine files like `m_at_socket7.c` implement `machine_common_init()` which chains device initialization
- Machines declare bus support via flags: `MACHINE_PCI`, `MACHINE_PS2_MCA`, `MACHINE_VLB`, etc.
- Internal device flags: `MACHINE_IDE_DUAL`, `MACHINE_PIIX4`, `MACHINE_SUPER_IO`, etc.

### Main Emulation Loop
Entry point: `src/86box.c` - initializes config, devices, starts emulation thread
- `cpu/` contains CPU emulation with optional dynarec (dynamic recompiler)
- `timer.c` provides microsecond-precision event scheduling
- `io.c` / `mem.c` handle I/O port and memory space registration

### Subsystem Organization
- `chipset/` - Motherboard chipsets (Intel PIIX, ALi, SiS, VIA, OPTi, etc.) - integrate PCI bridges, ISA-PCI bridges, IDE, USB, ACPI
- `video/` - Graphics cards (CGA, EGA, VGA, S3, Matrox, 3dfx Voodoo, ATi Mach64)
- `sound/` - Sound cards (Sound Blaster, AdLib, Gravis Ultrasound, AC97) 
- `disk/` + `floppy/` - Storage (hard disk, floppy controllers)
- `scsi/` - SCSI controllers and devices
- `network/` - Network adapters (NE2000, 3Com, RTL8139) with libslirp NAT backend
- `sio/` - Super I/O chips (combined FDC, parallel, serial, GPIO)

## Build System (CMake)

### Configuration
Use CMake presets (see `CMakePresets.json`):
```bash
cmake --preset regular       # Release build
cmake --preset debug         # Debug build
cmake --preset development   # Dev branch features enabled
```

Key CMake options in root `CMakeLists.txt`:
- `DEV_BRANCH` - Enables experimental devices (AMD_K5, G100, PCL printer, etc.)
- `NEW_DYNAREC` - Use PCem v15 dynarec (required for ARM64)
- `DYNAREC` - Enable dynamic recompiler (x86/x64 only by default)
- `QT` / `USE_QT6` - Qt5 or Qt6 UI (default Qt5)
- `STATIC_BUILD` - Static linking
- `OPENAL`, `FLUIDSYNTH`, `MUNT`, `VNC` - Optional audio/rendering backends

### Dependencies (vcpkg.json)
Core: SDL2, libpng, freetype, rtmidi, libslirp, fluidsynth
UI: Qt5/Qt6 (qtbase with opengl, vulkan, network, widgets features)
Optional: libmt32emu (MUNT), openal-soft

### Build Commands
```bash
cmake --preset debug
cmake --build build/debug
# Binary: build/debug/src/86Box
```

## Coding Conventions

### File Headers
Every file must include the standard GPL-2.0 header with author credits and copyright years (see `src/86box.c` lines 1-23)

### Style
- C11 standard (`#define HAVE_STDARG_H` is always present)
- Include order: standard C headers → platform headers → `<86box/86box.h>` → other 86box headers
- Use `UNUSED()` macro for unused parameters (defined in `src/include/86box/plat_unused.h`)
- Struct typedefs end with `_t`: `device_t`, `machine_t`, `piix_t`

### Memory Management
- Use `calloc()` for zero-initialized structures
- Device private data returned from `init()` functions
- Clean up in `close()` callback - free memory, close file handles

### Platform Abstraction
- `src/unix/` and `src/qt/` for platform-specific code
- Use `plat_*` functions for file I/O (`plat_fopen64`, `plat_tempfile`)
- Use `nvr_path()` for config directory paths

## Development Workflows

### Adding New Device
1. Create device file in appropriate subsystem directory
2. Define `device_t` constant with callbacks and config
3. Implement `init()`, `close()`, `reset()` functions
4. Register I/O ports via `io_sethandler()` or PCI config space
5. Add to device table/menu (check `src/qt/` for UI integration)
6. For DEV_BRANCH devices: guard with `#ifdef DEV_BRANCH` and add cmake option

### Adding New Machine
1. Create `m_*.c` file in `src/machine/`
2. Implement machine-specific init function calling `machine_common_init()`
3. Add entries to `machine_types[]` and machine table in `machine_table.c`
4. Define bus flags and internal device flags
5. Add ROM entries to 86Box/roms repository

### Debugging
- Enable with `GDBSTUB=ON` CMake option for GDB remote debugging
- `MINITRACE=ON` enables Chrome tracing
- Log macros per-subsystem (check device source files for `*_log()` patterns)

## Important Caveats

### Threading
- Most emulation runs in a single thread (CPU, devices, timers)
- UI runs in separate Qt thread
- Use `thread_*` functions from `src/thread.cpp` for synchronization

### Timing Precision
- Emulator uses microsecond event scheduling via `timer.c`
- CPU speed affects all device timing - test at multiple speeds

### Bus Dependencies
Check device flags match machine bus capabilities - PCI devices need `MACHINE_PCI`, ISA16 needs `MACHINE_AT`, etc.

### ROM Files
Hardware requiring ROMs must have them in [86Box/roms repository](https://github.com/86Box/roms). ROM path set via `--rompath` or config.

## Testing
No formal test framework currently. Key validation:
- Boot MS-DOS, Windows 3.1/95/98, OS/2, Linux on relevant machines
- Verify device detection in guest OS Device Manager
- Check hardware-specific software (games, benchmarks, demos)
- Use unit tester device (`doc/specifications/86box-unit-tester.md`) for video regression testing

## Resources
- Documentation: https://86box.readthedocs.io/
- Build guide: https://86box.readthedocs.io/en/latest/dev/buildguide.html
- IRC: #86Box on irc.ringoflightning.net
- Discord: https://discord.gg/QXK9XTv
- ROM repository: https://github.com/86Box/roms
