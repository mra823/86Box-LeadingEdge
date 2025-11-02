# Leading Edge Model D RTC Testing

This directory contains tools for testing the Leading Edge Model D Real-Time Clock (RTC) emulation.

## RTC Overview

The Leading Edge Model D has an integrated MM58167 RTC chip at I/O port **0x300-0x31F**. This is non-standard (IBM AT uses 0x70-0x7F).

### Features:
- Battery-backed real-time clock
- Time: Hours, minutes, seconds (BCD format)
- Date: Month, day, year (BCD format, year is base-80)
- Alarm functionality with interrupts
- Periodic interrupts

## Testing Without DOS Drivers

Since MS-DOS 1.x doesn't include RTC drivers, you can test the RTC directly using the provided test program.

### RTCTEST.COM

A small DOS .COM program that reads and displays the RTC time/date.

**Usage:**
1. Copy `rtctest.com` to your DOS disk image
2. Boot the Leading Edge Model D in 86Box
3. Run: `RTCTEST.COM`

**Output:**
```
Leading Edge Model D RTC Test
MM58167 chip at I/O port 0x300

Time: 18:43:27
Date: 11/01/1985

Raw hex values:
  Time: 18:43:27
  Date: 11:01:05
```

The raw hex values show BCD encoding:
- Time: 0x18 = 18 hours, 0x43 = 43 minutes, 0x27 = 27 seconds
- Date: 0x11 = November, 0x01 = 1st day, 0x05 = 1985 (base-80: 85-80=5)

### Building RTCTEST.COM

If you want to rebuild from source:

**Option 1: Using the provided script (no assembler needed)**
```bash
bash build_rtctest.sh
```

**Option 2: Assemble from source (requires NASM)**
```bash
nasm -f bin rtctest.asm -o rtctest.com
```

**Option 3: Using MASM or TASM**
```bash
tasm rtctest.asm
tlink /t rtctest.obj
```

## Testing with DOS Drivers

For full functionality, you can use period-correct RTC drivers:

1. **CLOCK.SYS** - Leading Edge's official RTC driver
2. **CLKDVR.SYS** - Third-party RTC driver

Add to CONFIG.SYS:
```
DEVICE=CLOCK.SYS
```

Then DOS TIME and DATE commands will read/write to the RTC.

## RTC Register Map

The MM58167 uses direct port addressing at 0x300-0x31F:

| Port   | Register | Description                    |
|--------|----------|--------------------------------|
| 0x300  | 0        | Milliseconds counter           |
| 0x301  | 1        | Hundredths/tenths of seconds   |
| 0x302  | 2        | Seconds (BCD)                  |
| 0x303  | 3        | Minutes (BCD)                  |
| 0x304  | 4        | Hours (BCD)                    |
| 0x305  | 5        | Day of week (1-7 BCD)          |
| 0x306  | 6        | Day of month (1-31 BCD)        |
| 0x307  | 7        | Month (1-12 BCD)               |
| 0x308  | 8        | Alarm: milliseconds            |
| 0x309  | 9        | Alarm: hundredths/tenths       |
| 0x30A  | 10       | Alarm: seconds                 |
| 0x30B  | 11       | Alarm: minutes                 |
| 0x30C  | 12       | Alarm: hours                   |
| 0x30D  | 13       | Alarm: day of week             |
| 0x30E  | 14       | Alarm: day of month / **YEAR** |
| 0x30F  | 15       | Alarm: month                   |
| 0x310  | 16       | Interrupt status               |
| 0x311  | 17       | Interrupt control              |
| 0x312  | 18       | Reset counters                 |
| 0x313  | 19       | Reset RAM                      |
| 0x314  | 20       | Status bit                     |
| 0x315  | 21       | GO command                     |
| 0x316  | 22       | Standby interrupt              |
| 0x31F  | 31       | Test mode                      |

**Note:** The Leading Edge stores the year in register 14 (0x30E), which is normally the alarm day-of-month register. Year is base-80 BCD (1985 = 0x05).

## Important Notes

- **Port Conflict:** The RTC at 0x300 conflicts with XT-IDE controllers (which typically use 0x300-0x30F). This is historically accurate - users had to configure XT-IDE to alternate addresses.
- **Year Base-80:** The year is stored as (year - 1980) in BCD format. For example, 1985 is stored as 0x05.
- **BCD Format:** All time/date values are in Binary-Coded Decimal. Each nibble represents one decimal digit.

## Troubleshooting

**Problem:** RTCTEST shows all zeros or 0xFF values
- **Solution:** The RTC may not be initialized. Reboot the emulated machine.

**Problem:** XT-IDE doesn't work
- **Solution:** Configure XT-IDE to use port 0x320 or 0x360 instead of 0x300.

**Problem:** Time is wrong
- **Solution:** The RTC initializes to your host system time. Check your system clock.
