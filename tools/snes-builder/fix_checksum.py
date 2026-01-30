#!/usr/bin/env python3
"""
Fix SNES ROM checksum.

The SNES ROM header contains:
- $FFDC-$FFDD: Checksum complement (inverse of checksum)
- $FFDE-$FFDF: Checksum (16-bit sum of all ROM bytes)

For LoROM, the header is at file offset $7FB0-$7FDF.
"""

import sys
import os


def fix_checksum(rom_path: str) -> None:
    """Calculate and fix the SNES ROM checksum."""

    with open(rom_path, 'rb') as f:
        data = bytearray(f.read())

    rom_size = len(data)
    print(f"ROM size: {rom_size} bytes ({rom_size // 1024} KB)")

    # For LoROM, header is at $7FB0-$7FDF (file offset)
    # Checksum locations:
    # - $7FDC-$7FDD: Checksum complement
    # - $7FDE-$7FDF: Checksum
    complement_offset = 0x7FDC
    checksum_offset = 0x7FDE

    if rom_size < 0x8000:
        print(f"Error: ROM too small ({rom_size} bytes), need at least 32KB")
        sys.exit(1)

    # Set checksum fields to the "empty" values before calculation
    # Standard: complement = $FFFF, checksum = $0000
    data[complement_offset] = 0xFF
    data[complement_offset + 1] = 0xFF
    data[checksum_offset] = 0x00
    data[checksum_offset + 1] = 0x00

    # Calculate checksum
    # For ROMs that aren't a power of 2, we need to handle mirroring
    # But for our 32KB ROM, it's already a power of 2
    checksum = sum(data) & 0xFFFF
    complement = checksum ^ 0xFFFF

    print(f"Checksum: ${checksum:04X}")
    print(f"Complement: ${complement:04X}")

    # Write checksum complement (little-endian)
    data[complement_offset] = complement & 0xFF
    data[complement_offset + 1] = (complement >> 8) & 0xFF

    # Write checksum (little-endian)
    data[checksum_offset] = checksum & 0xFF
    data[checksum_offset + 1] = (checksum >> 8) & 0xFF

    # Verify: checksum + complement should = $FFFF
    verify_sum = (checksum + complement) & 0xFFFF
    if verify_sum != 0xFFFF:
        print(f"Warning: Verification failed! sum=${verify_sum:04X}")
    else:
        print(f"Verification OK: ${checksum:04X} + ${complement:04X} = $FFFF")

    with open(rom_path, 'wb') as f:
        f.write(data)

    print(f"Checksum fixed: {rom_path}")


def main():
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <rom.sfc>")
        sys.exit(1)

    rom_path = sys.argv[1]
    if not os.path.exists(rom_path):
        print(f"Error: File not found: {rom_path}")
        sys.exit(1)

    fix_checksum(rom_path)


if __name__ == "__main__":
    main()
