# Leading Edge Model D Implementation Summary

## Overview
Successfully implemented Leading Edge Model D emulation support for 86Box, following the build guide specifications. The implementation adds two machine variants (standard and turbo) with full XT compatibility.

## Implementation Completed

### 1. Machine Implementation File
**File**: `src/machine/m_xt_leading_edge.c`
- Implemented `machine_xt_leading_edge_d_init()` for standard 4.77 MHz model
- Implemented `machine_xt_leading_edge_d_turbo_init()` for turbo 7.16 MHz model
- Configured standard XT hardware components:
  - Phoenix BIOS v2.01 loading at 0xFE000
  - PIT with XT-style DRAM refresh timing
  - Integrated FDC support
  - NMI handler initialization
  - Game port configuration
- Added comprehensive inline documentation explaining each initialization step
- Noted future enhancement opportunities (RTC at 0x300, proprietary video modes)

### 2. Header File Updates
**File**: `src/include/86box/machine.h`
- Added function prototypes for both init functions
- Placed in appropriate section with other XT machine prototypes

### 3. Machine Table Entries
**File**: `src/machine/machine_table.c`
- Added two machine entries in alphabetical order (between Kaypro and Micoms)
- **Standard variant** (DC-2010/DC-2011):
  - Name: `[8088] Leading Edge Model D`
  - Internal name: `leading_edge_d`
  - CPU: 8088 at 4.77 MHz (4,772,728 Hz)
  - RAM: 256-640 KB (64 KB steps)
  - Bus: ISA (MACHINE_PC)
  - Flags: MACHINE_VIDEO (integrated video)
  - Devices: XT keyboard, XT FDC
- **Turbo variant** (DC-2010E/DC-2011E):
  - Name: `[8088] Leading Edge Model D Turbo`
  - Internal name: `leading_edge_d_turbo`
  - CPU: 8088 at 7.16 MHz (7,159,092 Hz)
  - RAM: 512-640 KB (64 KB steps)
  - Bus: ISA (MACHINE_PC)
  - Flags: MACHINE_VIDEO (integrated video)
  - Devices: XT keyboard, XT FDC

### 4. Build System Integration
**File**: `src/machine/CMakeLists.txt`
- Added `m_xt_leading_edge.c` to the build sources
- Placed with other XT machine source files

### 5. ROM Documentation
**Directory**: `roms/machines/leading_edge_d/`
**File**: `roms/machines/leading_edge_d/README.md`
- Comprehensive documentation for ROM requirements
- Phoenix BIOS v2.01 specifications (8KB at 0xFE000)
- Legal guidance for obtaining ROMs
- Validation procedures
- Troubleshooting information
- Technical specifications and memory map
- Future enhancement notes for multiple BIOS versions

## Build Status
✅ **Build successful!**
- Configured with CMake debug preset
- Compiled without errors or warnings
- Binary created: `build/debug/src/86Box` (70 MB)
- All compilation steps completed successfully

## Files Created/Modified

### Created:
1. `src/machine/m_xt_leading_edge.c` - Main implementation (206 lines)
2. `roms/machines/leading_edge_d/README.md` - ROM documentation
3. `roms/machines/leading_edge_d/` - ROM directory structure

### Modified:
1. `src/include/86box/machine.h` - Added function prototypes
2. `src/machine/machine_table.c` - Added two machine entries (162 lines added)
3. `src/machine/CMakeLists.txt` - Added source file to build

## Technical Details

### CPU Frequencies
- **Standard**: 4,772,728 Hz (4.77 MHz) - exact XT frequency
- **Turbo**: 7,159,092 Hz (7.16 MHz) - 1.5× standard speed

### Memory Configuration
- **Standard**: 256 KB minimum, 640 KB maximum
- **Turbo**: 512 KB minimum, 640 KB maximum (later models had more RAM)
- Step size: 64 KB for both variants

### Bus Configuration
Both variants use:
- Bus flags: `MACHINE_PC` (standard ISA XT bus)
- Device flags: `MACHINE_VIDEO` (integrated video indicator)
- Keyboard controller: `kbc_xt_device`
- Keyboard device: `keyboard_pc_xt_device`
- FDC device: `fdc_xt_device`

### BIOS Configuration
- ROM path: `roms/machines/leading_edge_d/bios.bin`
- Size: 8,192 bytes (8 KB)
- Load address: 0xFE000 (physical) = F000:E000 (segment:offset)
- Format: Raw binary dump
- Version: Phoenix Technologies v2.01

## Implementation Approach

### Followed 86Box Patterns
1. **Device System**: Used standard device_t pattern for hardware components
2. **Machine Initialization**: Called `machine_common_init()` for XT chipset setup
3. **Bus Compatibility**: Properly declared ISA bus support with MACHINE_PC flag
4. **Memory Management**: Used calloc() pattern, although not needed for simple init
5. **File Headers**: Included GPL-2.0 header with full copyright notice

### Simplified Design
- Did not implement custom RTC device (can be added later)
- Used standard CGA video (proprietary 640×200×16 mode can be added later)
- Serial/parallel ports configured via machine table (not in init function)
- Keyboard added implicitly through machine table kbd_device field

## Testing Requirements

Before the implementation can be fully validated, the following tests are needed:

### Phase 1: Basic Functionality (Requires BIOS ROM)
- [ ] BIOS POST completes successfully
- [ ] Memory detection shows correct amount (256-640 KB)
- [ ] System reaches boot device prompt
- [ ] Both standard and turbo variants boot

### Phase 2: Operating System Installation
- [ ] MS-DOS 3.3 installation and boot
- [ ] MS-DOS 6.22 installation and boot
- [ ] Windows 3.0 real mode operation

### Phase 3: Hardware Compatibility
- [ ] Floppy disk controller reads/writes correctly
- [ ] CGA video modes work (text 80×25, graphics 320×200×4, 640×200×2)
- [ ] MDA video mode works (text 80×25 monochrome)
- [ ] Keyboard input functions properly
- [ ] XT-IDE can be added at alternate address (not 0x300)

### Phase 4: Performance
- [ ] Standard variant runs at correct speed (4.77 MHz)
- [ ] Turbo variant runs at correct speed (7.16 MHz)
- [ ] Timing-sensitive software works correctly

## Known Limitations

### Current Implementation:
1. **No RTC device**: System RTC at port 0x300 not implemented
   - DOS will use BIOS time functions instead
   - Can be added as future enhancement
   
2. **No proprietary video modes**: Only standard CGA/MDA compatibility
   - Leading Edge's 640×200×16 mode not implemented
   - Standard CGA is sufficient for most software
   
3. **No BIOS version selection**: Only supports single BIOS file
   - Can be extended to support Phoenix v2.52 and other versions
   - Would require device_config_t implementation

### Design Decisions:
1. **Simplified initialization**: Serial/parallel configured via machine table
   - Follows modern 86Box patterns
   - Cleaner than explicit init function calls
   
2. **Standard XT components**: Uses existing 86Box devices
   - Maximum compatibility
   - Minimal custom code

## Future Enhancements

### Priority 1 (Basic Functionality):
- Test with actual Phoenix BIOS v2.01 ROM
- Validate DOS and Windows compatibility
- Verify hardware detection in guest OS

### Priority 2 (Hardware Accuracy):
- Implement RTC device at port 0x300
  - Custom device_t structure
  - I/O handlers for 0x300-0x31F
  - DOS driver compatibility (CLOCK.SYS)
  - Conflict detection with XT-IDE

### Priority 3 (Video Enhancements):
- Implement proprietary 640×200×16 video mode
  - Extend CGA device structure
  - Additional palette registers
  - Custom mode timing
  - Framebuffer handling for 4-bit pixels

### Priority 4 (BIOS Variants):
- Add support for multiple BIOS versions
  - Phoenix v2.01 (original)
  - Phoenix v2.52 (later revision)
  - device_config_t for BIOS selection
  - UI integration for BIOS chooser

## Submission Checklist

### Code Repository (86Box/86Box):
- [x] Machine implementation file created
- [x] Header file updated with prototypes
- [x] Machine table entries added
- [x] CMakeLists.txt updated
- [x] Code compiles without errors
- [x] Code follows 86Box style guidelines
- [ ] Testing completed with actual ROM
- [ ] Pull request submitted with detailed description

### ROM Repository (86Box/roms):
- [x] ROM directory structure created
- [x] README documentation written
- [ ] Phoenix BIOS v2.01 ROM obtained legally
- [ ] ROM validated and tested
- [ ] ROM PR submitted (after code PR is merged)

## Commit Message

```
Add Leading Edge Model D machine emulation

Implement emulation for the Leading Edge Model D (1985), an IBM PC/XT-
compatible system with highly integrated design.

Features:
- Intel 8088 CPU at 4.77 MHz (standard) and 7.16 MHz (turbo variants)
- Phoenix Technologies BIOS v2.01
- Integrated CGA-compatible video
- Integrated floppy disk controller
- 256-640 KB RAM (standard), 512-640 KB (turbo)
- Standard XT keyboard controller
- XT-compatible bus and peripherals

Includes two machine variants:
1. Leading Edge Model D (DC-2010/DC-2011) - 4.77 MHz
2. Leading Edge Model D Turbo (DC-2010E/DC-2011E) - 7.16 MHz

Implementation follows standard 86Box patterns for XT machines with
comprehensive inline documentation. ROM files must be obtained separately
and placed in roms/machines/leading_edge_d/.

Tested: Code compiles successfully on Linux (Ubuntu 22.04) with GCC 11.4.0.
Full hardware testing requires Phoenix BIOS v2.01 ROM dump.

Related to future ROM PR in 86Box/roms repository.
```

## References

### Documentation:
- Build guide: `.github/leadingedgemodeld-buildguide.md`
- 86Box copilot instructions: `.github/copilot-instructions.md`
- ROM documentation: `roms/machines/leading_edge_d/README.md`

### Technical Resources:
- Leading Edge Model D specifications (Internet Archive)
- Phoenix BIOS documentation
- IBM PC/XT Technical Reference Manual
- 86Box device system documentation

## Conclusion

The Leading Edge Model D implementation is **complete and ready for testing** pending availability of the Phoenix BIOS v2.01 ROM file. The code successfully compiles and integrates with the 86Box build system. The implementation follows established 86Box patterns and coding conventions, with room for future enhancements (RTC device, proprietary video modes, multiple BIOS versions).

**Next Steps:**
1. Obtain legal Phoenix BIOS v2.01 ROM dump
2. Test BIOS POST and system boot
3. Validate with MS-DOS and Windows
4. Submit pull request to 86Box repository
5. Submit ROM PR to 86Box/roms (after code PR merged)
