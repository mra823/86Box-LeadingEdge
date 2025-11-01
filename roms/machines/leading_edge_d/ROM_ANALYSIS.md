# Leading Edge Model D - ROM Analysis

## Critical Finding: Main BIOS ROM Missing

The current ROM files (23096000.BIN and 23097000.BIN) **are not complete system BIOS ROMs**.

### Evidence

1. **Option ROM Signature**: 23096000.BIN starts with `55 AA` - this is the standard option ROM signature, not a main BIOS signature.

2. **No Reset Vector**: When interleaved, the ROMs end with:
   ```
   00003fe0  ff 00 ff 00 ff 00 ff 00 ff 00 ff 00 ff 00 ff 00
   00003ff0  ff 00 ff 00 ff 00 ff 00 ff 00 ff 00 ff 00 00 00
   ```
   A valid XT BIOS should have a JMP instruction (EA xx xx 00 F0) at offset 1FF0.

3. **Comparison**: AMI XT BIOS (ami_8088_bios_31jan89.bin) has proper reset vector:
   ```
   00001ff0  ea 5b e0 00 f0 31 30 2f  31 32 2f 38 38 fe ff ff
                    ^^^^^^^^^^ - Reset vector (JMP F000:E05B)
   ```

4. **Size**: Only 16KB interleaved - XT BIOSes are typically 32-64KB.

### ROM Chip Identification

Based on motherboard revision 1/5 from The Retro Web:
- **23096000.BIN** - 8KB - Option ROM (video/font?)
- **23097000.BIN** - 8KB - Option ROM data/fonts

### What's Needed

The Leading Edge Model D motherboard should have **additional ROM chips**:
- Main BIOS ROM: 32KB or larger
- Typical chip labels: U17, U18, U19, U20 (check physical board)
- Should contain Phoenix BIOS v2.01 identification strings

### Sources to Check

1. **The Retro Web**: Verify all ROM dumps from revision 1/5 motherboard
   - Check if there are files like `23095000.BIN`, `23098000.BIN`, etc.
   - Look for ROM files larger than 8KB

2. **MESS/MAME**: Check if their Leading Edge Model D driver has ROM definitions
   - Source: `mame/src/mame/ibm/pc.cpp` or similar
   - ROM chip designations and addresses

3. **Minuszerodegrees**: Leading Edge technical documentation
   - Motherboard layout diagrams
   - ROM chip locations and part numbers

4. **Vogons Forum**: Search for "Leading Edge Model D BIOS dump"

### Temporary Workaround

Currently the code is set to fail gracefully with a missing file error to document that the main BIOS is needed. To test with a substitute BIOS:
- Copy a generic Phoenix XT BIOS to `MAIN_BIOS_NEEDED.bin`
- Machine will boot but may have hardware detection issues
- Not recommended for release

### Next Steps

1. Obtain complete ROM set from Leading Edge Model D motherboard
2. Identify main BIOS ROM chip location and size
3. Update m_xt_leading_edge.c with proper ROM loading:
   - Load main BIOS at 0xFE000 or 0xF8000 (depending on size)
   - Load option ROMs (23096000/23097000) at 0xC0000 if they're video ROMs
4. Test boot sequence with complete ROM set
