"""Shared ROM utilities for SNES cartridge building.

This module provides common functionality for creating and manipulating
SNES ROM images, including header creation, checksum calculation, and
cartridge type constants.
"""

from enum import IntEnum
from typing import Tuple


class CartType(IntEnum):
    """SNES cartridge types (header byte at $FFD6)."""
    ROM_ONLY = 0x00
    ROM_RAM = 0x01
    ROM_RAM_BATTERY = 0x02
    SUPERFX = 0x13
    SUPERFX_RAM = 0x14
    SUPERFX_RAM_BATTERY = 0x15
    SA1 = 0x34
    SA1_RAM = 0x35
    SA1_RAM_BATTERY = 0x36


class MapMode(IntEnum):
    """SNES memory map modes (header byte at $FFD5)."""
    LOROM = 0x20
    HIROM = 0x21
    LOROM_FAST = 0x30
    HIROM_FAST = 0x31
    EXLOROM = 0x22
    EXHIROM = 0x25


class ROMSize(IntEnum):
    """ROM size codes (header byte at $FFD7).

    Value is log2(size in KB) - 10, so $08 = 256KB.
    """
    SIZE_32KB = 0x05    # 2^15 = 32KB
    SIZE_64KB = 0x06    # 2^16 = 64KB
    SIZE_128KB = 0x07   # 2^17 = 128KB
    SIZE_256KB = 0x08   # 2^18 = 256KB
    SIZE_512KB = 0x09   # 2^19 = 512KB
    SIZE_1MB = 0x0A     # 2^20 = 1MB
    SIZE_2MB = 0x0B     # 2^21 = 2MB
    SIZE_4MB = 0x0C     # 2^22 = 4MB


class RAMSize(IntEnum):
    """RAM size codes (header byte at $FFD8).

    Value is log2(size in KB), so $05 = 32KB.
    """
    NONE = 0x00
    SIZE_2KB = 0x01
    SIZE_4KB = 0x02
    SIZE_8KB = 0x03
    SIZE_16KB = 0x04
    SIZE_32KB = 0x05
    SIZE_64KB = 0x06
    SIZE_128KB = 0x07


# Header offsets (relative to header base, which is $FFC0 for LoROM)
HEADER_TITLE_OFFSET = 0x00        # $FFC0-$FFD4: 21-byte title
HEADER_MAP_MODE_OFFSET = 0x15     # $FFD5: Map mode
HEADER_TYPE_OFFSET = 0x16         # $FFD6: Cartridge type
HEADER_ROM_SIZE_OFFSET = 0x17     # $FFD7: ROM size
HEADER_RAM_SIZE_OFFSET = 0x18     # $FFD8: RAM size
HEADER_REGION_OFFSET = 0x19       # $FFD9: Region code
HEADER_DEVELOPER_OFFSET = 0x1A    # $FFDA: Developer ID
HEADER_VERSION_OFFSET = 0x1B      # $FFDB: Version
HEADER_CHECKSUM_COMP_OFFSET = 0x1C  # $FFDC-$FFDD: Checksum complement
HEADER_CHECKSUM_OFFSET = 0x1E     # $FFDE-$FFDF: Checksum
HEADER_RESET_VECTOR_OFFSET = 0x3C # $FFFC-$FFFD: Reset vector (native mode)

# Standard header locations
LOROM_HEADER_OFFSET = 0x7FC0      # LoROM header at file offset
HIROM_HEADER_OFFSET = 0xFFC0      # HiROM header at file offset

# Minimum sizes
MIN_ROM_SIZE = 32 * 1024          # 32KB minimum for any SNES ROM
MIN_SUPERFX_ROM_SIZE = 256 * 1024  # 256KB minimum for SuperFX


def calculate_checksum(data: bytes, zero_checksum_area: bool = True) -> int:
    """Calculate SNES ROM checksum.

    The checksum is the 16-bit sum of all ROM bytes.

    Args:
        data: ROM data bytes
        zero_checksum_area: If True, treat the checksum bytes as zero
                           for calculation purposes

    Returns:
        16-bit checksum value
    """
    data_list = list(data)

    if zero_checksum_area and len(data) >= LOROM_HEADER_OFFSET + HEADER_CHECKSUM_OFFSET + 2:
        # Zero out checksum area for calculation (LoROM location)
        comp_off = LOROM_HEADER_OFFSET + HEADER_CHECKSUM_COMP_OFFSET
        check_off = LOROM_HEADER_OFFSET + HEADER_CHECKSUM_OFFSET
        data_list[comp_off] = 0
        data_list[comp_off + 1] = 0
        data_list[check_off] = 0
        data_list[check_off + 1] = 0

    return sum(data_list) & 0xFFFF


def calculate_checksum_complement(checksum: int) -> int:
    """Calculate checksum complement (XOR with $FFFF).

    Args:
        checksum: 16-bit checksum value

    Returns:
        16-bit complement value
    """
    return checksum ^ 0xFFFF


def create_header(
    title: str = "SNES PROGRAM",
    map_mode: int = MapMode.LOROM,
    cart_type: int = CartType.ROM_ONLY,
    rom_size: int = ROMSize.SIZE_256KB,
    ram_size: int = RAMSize.NONE,
    region: int = 0x01,  # North America
    developer: int = 0x00,
    version: int = 0x00,
    reset_vector: int = 0x8000,
) -> bytes:
    """Create SNES ROM header bytes.

    Creates a 64-byte header block suitable for writing at the header
    location ($7FC0 for LoROM, $FFC0 for HiROM).

    Args:
        title: Game title (max 21 characters, padded with spaces)
        map_mode: Memory map mode (LoROM, HiROM, etc.)
        cart_type: Cartridge type (ROM only, SuperFX, SA1, etc.)
        rom_size: ROM size code
        ram_size: RAM size code
        region: Region code (0x01 = North America, 0x02 = Europe, etc.)
        developer: Developer ID
        version: ROM version
        reset_vector: Address to jump to on reset

    Returns:
        64 bytes of header data
    """
    header = bytearray(64)

    # Title (21 bytes, padded with spaces)
    title_bytes = title.encode('ascii', errors='replace')[:21]
    title_padded = title_bytes.ljust(21, b' ')
    header[HEADER_TITLE_OFFSET:HEADER_TITLE_OFFSET + 21] = title_padded

    # Map mode
    header[HEADER_MAP_MODE_OFFSET] = map_mode

    # Cartridge type
    header[HEADER_TYPE_OFFSET] = cart_type

    # ROM size
    header[HEADER_ROM_SIZE_OFFSET] = rom_size

    # RAM size
    header[HEADER_RAM_SIZE_OFFSET] = ram_size

    # Region
    header[HEADER_REGION_OFFSET] = region

    # Developer ID
    header[HEADER_DEVELOPER_OFFSET] = developer

    # Version
    header[HEADER_VERSION_OFFSET] = version

    # Checksum complement (placeholder - will be calculated later)
    header[HEADER_CHECKSUM_COMP_OFFSET] = 0xFF
    header[HEADER_CHECKSUM_COMP_OFFSET + 1] = 0xFF

    # Checksum (placeholder - will be calculated later)
    header[HEADER_CHECKSUM_OFFSET] = 0x00
    header[HEADER_CHECKSUM_OFFSET + 1] = 0x00

    # Vectors (at offset 0x20-0x3F)
    # Native mode vectors (we only set reset)
    # Reset vector at $FFFC-$FFFD (offset 0x3C-0x3D)
    header[HEADER_RESET_VECTOR_OFFSET] = reset_vector & 0xFF
    header[HEADER_RESET_VECTOR_OFFSET + 1] = (reset_vector >> 8) & 0xFF

    return bytes(header)


def write_header(rom: bytearray, header: bytes, offset: int = LOROM_HEADER_OFFSET) -> None:
    """Write header bytes to ROM at specified offset.

    Args:
        rom: ROM data (modified in place)
        header: Header bytes to write
        offset: Offset to write header at
    """
    for i, byte in enumerate(header):
        rom[offset + i] = byte


def write_checksum(rom: bytearray, offset: int = LOROM_HEADER_OFFSET) -> Tuple[int, int]:
    """Calculate and write checksum to ROM.

    Args:
        rom: ROM data (modified in place)
        offset: Header offset

    Returns:
        Tuple of (checksum, complement)
    """
    # Clear checksum bytes
    comp_off = offset + HEADER_CHECKSUM_COMP_OFFSET
    check_off = offset + HEADER_CHECKSUM_OFFSET
    rom[comp_off] = 0
    rom[comp_off + 1] = 0
    rom[check_off] = 0
    rom[check_off + 1] = 0

    # Calculate checksum
    checksum = sum(rom) & 0xFFFF
    complement = checksum ^ 0xFFFF

    # Write complement (little-endian)
    rom[comp_off] = complement & 0xFF
    rom[comp_off + 1] = (complement >> 8) & 0xFF

    # Write checksum (little-endian)
    rom[check_off] = checksum & 0xFF
    rom[check_off + 1] = (checksum >> 8) & 0xFF

    return checksum, complement


def fix_checksum(rom_path: str, verbose: bool = False) -> Tuple[int, int]:
    """Fix checksum in a ROM file.

    Args:
        rom_path: Path to ROM file
        verbose: If True, print status messages

    Returns:
        Tuple of (checksum, complement)

    Raises:
        ValueError: If ROM is too small
    """
    with open(rom_path, 'rb') as f:
        data = bytearray(f.read())

    rom_size = len(data)

    if verbose:
        print(f"ROM size: {rom_size} bytes ({rom_size // 1024} KB)")

    if rom_size < MIN_ROM_SIZE:
        raise ValueError(f"ROM too small ({rom_size} bytes), need at least {MIN_ROM_SIZE}")

    # Calculate and write checksum
    checksum, complement = write_checksum(data, LOROM_HEADER_OFFSET)

    if verbose:
        print(f"Checksum: ${checksum:04X}")
        print(f"Complement: ${complement:04X}")

        # Verify
        verify_sum = (checksum + complement) & 0xFFFF
        if verify_sum != 0xFFFF:
            print(f"Warning: Verification failed! sum=${verify_sum:04X}")
        else:
            print(f"Verification OK: ${checksum:04X} + ${complement:04X} = $FFFF")

    with open(rom_path, 'wb') as f:
        f.write(data)

    if verbose:
        print(f"Checksum fixed: {rom_path}")

    return checksum, complement


def pad_rom(rom: bytearray, min_size: int, fill_byte: int = 0xFF) -> None:
    """Pad ROM to minimum size.

    Args:
        rom: ROM data (modified in place)
        min_size: Minimum size in bytes
        fill_byte: Byte value to fill with
    """
    current_size = len(rom)
    if current_size < min_size:
        rom.extend([fill_byte] * (min_size - current_size))


def get_rom_size_code(size_bytes: int) -> int:
    """Get ROM size code for a given size in bytes.

    Args:
        size_bytes: ROM size in bytes

    Returns:
        ROM size code for header
    """
    # Find the smallest power of 2 that fits
    size_kb = size_bytes // 1024

    if size_kb <= 32:
        return ROMSize.SIZE_32KB
    elif size_kb <= 64:
        return ROMSize.SIZE_64KB
    elif size_kb <= 128:
        return ROMSize.SIZE_128KB
    elif size_kb <= 256:
        return ROMSize.SIZE_256KB
    elif size_kb <= 512:
        return ROMSize.SIZE_512KB
    elif size_kb <= 1024:
        return ROMSize.SIZE_1MB
    elif size_kb <= 2048:
        return ROMSize.SIZE_2MB
    else:
        return ROMSize.SIZE_4MB
