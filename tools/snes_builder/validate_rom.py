#!/usr/bin/env python3
"""
Validate SNES ROM header and report any issues.
"""

import sys

def validate_rom(rom_path):
    with open(rom_path, 'rb') as f:
        rom = f.read()

    rom_size = len(rom)
    print(f"ROM size: {rom_size} bytes ({rom_size // 1024}KB)")

    # For LoROM, header is at 0x7FB0 (for 32KB ROM mapped at $8000)
    # Header offset = 0x7FB0 for LoROM, 0xFFB0 for HiROM
    header_offset = 0x7FB0

    if rom_size < header_offset + 0x50:
        print(f"ERROR: ROM too small for header at 0x{header_offset:04X}")
        return False

    # Read header fields
    maker_code = rom[header_offset:header_offset+2].decode('ascii', errors='replace')
    game_code = rom[header_offset+2:header_offset+6].decode('ascii', errors='replace')

    title_offset = header_offset + 0x10  # 0x7FC0
    title = rom[title_offset:title_offset+21].decode('ascii', errors='replace')

    map_mode = rom[header_offset + 0x25]  # 0x7FD5
    cart_type = rom[header_offset + 0x26]  # 0x7FD6
    rom_size_byte = rom[header_offset + 0x27]  # 0x7FD7
    ram_size = rom[header_offset + 0x28]  # 0x7FD8
    country = rom[header_offset + 0x29]  # 0x7FD9
    dev_id = rom[header_offset + 0x2A]  # 0x7FDA
    version = rom[header_offset + 0x2B]  # 0x7FDB

    checksum_comp = rom[header_offset + 0x2C] | (rom[header_offset + 0x2D] << 8)
    checksum = rom[header_offset + 0x2E] | (rom[header_offset + 0x2F] << 8)

    # Interrupt vectors
    reset_vector = rom[0x7FFC] | (rom[0x7FFD] << 8)
    nmi_vector = rom[0x7FEA] | (rom[0x7FEB] << 8)

    print(f"\n=== SNES ROM Header Analysis ===")
    print(f"Header offset: 0x{header_offset:04X}")
    print(f"Maker code: '{maker_code}'")
    print(f"Game code: '{game_code}'")
    print(f"Title (21 bytes): '{title}'")
    print(f"  Title bytes: {' '.join(f'{b:02X}' for b in rom[title_offset:title_offset+21])}")
    print(f"Map mode: 0x{map_mode:02X}", end="")

    errors = []
    warnings = []

    # Validate map mode
    if map_mode == 0x20:
        print(" (LoROM)")
    elif map_mode == 0x21:
        print(" (HiROM)")
    elif map_mode == 0x30:
        print(" (LoROM + FastROM)")
    elif map_mode == 0x31:
        print(" (HiROM + FastROM)")
    else:
        print(f" (INVALID - expected 0x20/0x21/0x30/0x31)")
        errors.append(f"Invalid map mode 0x{map_mode:02X}")

    print(f"Cartridge type: 0x{cart_type:02X}")
    print(f"ROM size: 0x{rom_size_byte:02X} ({(1 << rom_size_byte)} KB declared)")
    print(f"RAM size: 0x{ram_size:02X}")
    print(f"Country: 0x{country:02X}")
    print(f"Developer ID: 0x{dev_id:02X}")
    print(f"Version: 0x{version:02X}")
    print(f"Checksum complement: 0x{checksum_comp:04X}")
    print(f"Checksum: 0x{checksum:04X}")
    print(f"Checksum + complement: 0x{(checksum + checksum_comp) & 0xFFFF:04X}", end="")

    if (checksum + checksum_comp) & 0xFFFF == 0xFFFF:
        print(" (VALID)")
    else:
        print(" (INVALID - should be 0xFFFF)")
        errors.append("Checksum + complement != 0xFFFF")

    # Calculate actual checksum
    actual_checksum = sum(rom) & 0xFFFF
    # Subtract the checksum bytes themselves (they were 0 when checksum was calculated)
    actual_checksum = (actual_checksum - checksum - checksum_comp) & 0xFFFF
    # The stored checksum should match
    print(f"Calculated checksum: 0x{actual_checksum:04X}", end="")
    if actual_checksum == checksum:
        print(" (MATCHES)")
    else:
        print(f" (MISMATCH with stored 0x{checksum:04X})")
        warnings.append("Checksum mismatch (may be OK if fixer ran)")

    print(f"\nReset vector: 0x{reset_vector:04X}")
    print(f"NMI vector: 0x{nmi_vector:04X}")

    # Validate reset vector
    if reset_vector < 0x8000 or reset_vector > 0xFFFF:
        errors.append(f"Reset vector 0x{reset_vector:04X} outside ROM range")

    # Check for common title issues
    title_bytes = rom[title_offset:title_offset+21]
    if any(b < 0x20 or b > 0x7E for b in title_bytes):
        warnings.append("Title contains non-printable characters")

    # Check byte after title (should be map mode, not part of title)
    byte_after_title = rom[title_offset + 21]
    if byte_after_title == 0x20:
        # Could be valid LoROM or an extra space in title
        print(f"\nByte at 0x7FD5 (map mode position): 0x{byte_after_title:02X} (space or LoROM)")

    print(f"\n=== Validation Results ===")
    if errors:
        print("ERRORS:")
        for e in errors:
            print(f"  - {e}")
    if warnings:
        print("WARNINGS:")
        for w in warnings:
            print(f"  - {w}")
    if not errors and not warnings:
        print("ROM header appears valid!")

    return len(errors) == 0

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <rom.sfc>")
        sys.exit(1)

    valid = validate_rom(sys.argv[1])
    sys.exit(0 if valid else 1)
