# Leading Edge Model D BIOS ROMs

This directory contains BIOS ROM files for the Leading Edge Model D emulation in 86Box.

## Required ROM Files

### bios.bin
- **Description**: Phoenix Technologies BIOS v2.01
- **Size**: 8,192 bytes (8 KB)
- **Load Address**: 0xFE000 (segment F000:E000)
- **Format**: Raw binary dump
- **Models**: Leading Edge Model D (DC-2010, DC-2011) and Model D Turbo (DC-2010E, DC-2011E)

## Obtaining ROM Files

The Leading Edge Model D BIOS ROM must be obtained legally through one of these methods:

1. **Dump from actual hardware**: Use an EPROM programmer (Willem, TL866, etc.) to read the BIOS chip from a physical Leading Edge Model D system
2. **Software dump**: Use DEBUG.COM or similar tools on real hardware to dump the ROM to a file
3. **Internet Archive**: Search for "Leading Edge Model D" BIOS files (verify licensing and authenticity)
4. **Vintage computing communities**: Check forums like VCFed for legally shared ROM dumps

## ROM Validation

Before using a ROM file with 86Box, verify:

1. **File size is exactly 8192 bytes**:
   ```bash
   ls -l bios.bin
   # Should show: -rw-r--r-- 1 user user 8192 ...
   ```

2. **Contains valid x86 code**: The ROM should start with valid 8086/8088 instructions

3. **Contains Phoenix BIOS signature**: Search for "Phoenix" string in the ROM:
   ```bash
   strings bios.bin | grep -i phoenix
   ```

4. **Test in emulator**: The ROM should allow the system to POST and reach the boot device prompt

## ROM Versions

Different Leading Edge Model D systems may have different BIOS versions. Known versions include:

- **Phoenix v2.01**: Most common version, used in standard DC-2010/DC-2011 models
- **Phoenix v2.52**: Later version found in some E models (DC-2010E/DC-2011E)

If you have multiple BIOS versions, name them descriptively:
- `phoenix_v201.bin` - Phoenix BIOS v2.01
- `phoenix_v252.bin` - Phoenix BIOS v2.52

Update the machine initialization code in `src/machine/m_xt_leading_edge.c` to support multiple BIOS files if needed.

## Technical Specifications

### Memory Map
```
0xF0000 - 0xFDFFF   Unused (mirror space)
0xFE000 - 0xFFFFF   BIOS ROM (8 KB at FE000h)
```

### BIOS Features
- IBM PC/XT compatible POST (Power-On Self Test)
- Support for 256-640 KB RAM (some versions support up to 768 KB)
- Integrated CGA and MDA video support
- Floppy disk controller initialization
- Serial and parallel port configuration
- Real-time clock support (via DOS driver)
- XT-compatible keyboard interface

## Troubleshooting

### ROM won't load
- Verify file path: `roms/machines/leading_edge_d/bios.bin`
- Check file permissions (must be readable)
- Ensure file is exactly 8192 bytes
- Verify ROM file is not corrupted

### System won't boot
- Verify video card is configured (CGA or MDA)
- Check that floppy disk controller is set to "Internal"
- Ensure boot media (floppy or hard disk) is configured
- Check 86Box log file for error messages

### POST errors
- Memory errors: Verify RAM is set between 256-640 KB
- Keyboard errors: Normal for XT systems without keyboard attached
- Disk errors: Expected if no boot media configured

## Contributing ROM Files

If you have a legally obtained Leading Edge Model D BIOS ROM that you'd like to contribute:

1. Verify the ROM is working and validated
2. Create documentation with ROM version, source, and verification details
3. Submit to the 86Box/roms repository (https://github.com/86Box/roms)
4. Reference the code PR that added Leading Edge Model D support

Note: ROM contributions to the 86Box/roms repository are only accepted after the corresponding emulation code has been merged into the main 86Box repository.

## License and Legal

BIOS ROM files are copyrighted by their respective manufacturers (Phoenix Technologies in this case). Users must obtain ROM files legally and in compliance with applicable copyright laws. 86Box developers do not provide ROM files and cannot assist with obtaining them.

## Additional Resources

- Leading Edge Model D Technical Documentation: Internet Archive
- Phoenix BIOS Documentation: Various vintage computing archives
- 86Box Documentation: https://86box.readthedocs.io/
- ROM dumping guides: http://www.minuszerodegrees.net/

## Support

For help with Leading Edge Model D emulation:
- 86Box Discord: https://discord.gg/QXK9XTv
- 86Box IRC: #86Box on irc.ringoflightning.net
- GitHub Issues: https://github.com/86Box/86Box/issues
