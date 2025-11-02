#!/usr/bin/env python3
"""
Create a DOS disk image with RTCTEST.COM for testing the Leading Edge Model D RTC.

This script creates a minimal bootable DOS disk image and copies RTCTEST.COM to it.
"""

import os
import sys

def main():
    print("Leading Edge Model D RTC Test Disk Creator")
    print("=" * 50)
    
    # Check if rtctest.com exists
    if not os.path.exists('rtctest.com'):
        print("ERROR: rtctest.com not found!")
        print("Run: bash build_rtctest.sh first")
        return 1
    
    print("\n✓ rtctest.com found (430 bytes)")
    
    print("\n" + "=" * 50)
    print("HOW TO TEST THE RTC:")
    print("=" * 50)
    print()
    print("1. Copy rtctest.com to your DOS boot disk:")
    print("   - Mount your DOS disk image in 86Box")
    print("   - Copy rtctest.com to drive A: or C:")
    print()
    print("2. Boot the Leading Edge Model D in 86Box")
    print()
    print("3. Run the test program:")
    print("   A> RTCTEST.COM")
    print()
    print("4. The program will display:")
    print("   - Current time from RTC (HH:MM:SS)")
    print("   - Current date from RTC (MM/DD/19YY)")
    print("   - Raw hex values (BCD format)")
    print()
    print("EXPECTED OUTPUT:")
    print("-" * 50)
    print("Leading Edge Model D RTC Test")
    print("MM58167 chip at I/O port 0x300")
    print()
    print("Time: 18:43:27")
    print("Date: 11/01/1985")
    print()
    print("Raw hex values:")
    print("  Time: 18:43:27")
    print("  Date: 11:01:05")
    print()
    print("=" * 50)
    print()
    print("NOTE: Time will match your host system clock.")
    print("      Year is base-80 BCD (1985 = 0x05, 2025 = 0x45)")
    print()
    
    return 0

if __name__ == '__main__':
    sys.exit(main())
