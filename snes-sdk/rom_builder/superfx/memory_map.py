"""GSU (SuperFX) memory map utilities.

This module provides functions for address translation between SNES and GSU
address spaces, memory region detection, and screen buffer calculations.
"""

from typing import Tuple

# =============================================================================
# Memory Region Constants
# =============================================================================

# ROM region (LoROM: $8000-$FFFF per bank)
ROM_START = 0x8000
ROM_END = 0xFFFF

# RAM region (full 64KB per bank)
RAM_START = 0x0000
RAM_END = 0xFFFF

# GSU register region
REGISTER_START = 0x3000
REGISTER_END = 0x32FF

# Special banks
RAM_BANK_0 = 0x70  # First 64KB of cart RAM
RAM_BANK_1 = 0x71  # Second 64KB of cart RAM

# LoROM banks
LOROM_BANK_START = 0x00
LOROM_BANK_END = 0x3F

# HiROM banks
HIROM_BANK_START = 0x40
HIROM_BANK_END = 0x5F


# =============================================================================
# Address Translation
# =============================================================================

def snes_to_gsu_rom(bank: int, addr: int) -> Tuple[int, int]:
    """Translate SNES ROM address to GSU address.

    For LoROM: Banks $00-$3F, addresses $8000-$FFFF
    For HiROM: Banks $40-$5F, addresses $0000-$FFFF

    Args:
        bank: SNES bank number
        addr: SNES address within bank

    Returns:
        Tuple of (GSU bank, GSU address)
    """
    return (bank, addr)


def snes_to_gsu_ram(bank: int, addr: int) -> Tuple[int, int]:
    """Translate SNES RAM address to GSU address.

    RAM is at banks $70-$71, full 64KB per bank.

    Args:
        bank: SNES bank number ($70 or $71)
        addr: SNES address within bank

    Returns:
        Tuple of (GSU bank, GSU address)
    """
    return (bank, addr)


def gsu_to_snes_rom(bank: int, addr: int) -> Tuple[int, int]:
    """Translate GSU ROM address to SNES address.

    Args:
        bank: GSU ROM bank
        addr: GSU address

    Returns:
        Tuple of (SNES bank, SNES address)
    """
    return (bank, addr)


def gsu_to_snes_ram(bank: int, addr: int) -> Tuple[int, int]:
    """Translate GSU RAM address to SNES address.

    Args:
        bank: GSU RAM bank ($70 or $71)
        addr: GSU address

    Returns:
        Tuple of (SNES bank, SNES address)
    """
    return (bank, addr)


# =============================================================================
# Memory Region Detection
# =============================================================================

def is_rom_address(bank: int, addr: int) -> bool:
    """Check if an address is in ROM space.

    LoROM: Banks $00-$3F, addresses $8000-$FFFF
    HiROM: Banks $40-$5F, addresses $0000-$FFFF

    Args:
        bank: Bank number
        addr: Address within bank

    Returns:
        True if address is in ROM space
    """
    # LoROM region
    if LOROM_BANK_START <= bank <= LOROM_BANK_END:
        return ROM_START <= addr <= ROM_END

    # HiROM region
    if HIROM_BANK_START <= bank <= HIROM_BANK_END:
        return True

    return False


def is_ram_address(bank: int, addr: int) -> bool:
    """Check if an address is in RAM space.

    RAM is at banks $70-$71.

    Args:
        bank: Bank number
        addr: Address within bank

    Returns:
        True if address is in RAM space
    """
    return bank in (RAM_BANK_0, RAM_BANK_1)


def is_register_address(bank: int, addr: int) -> bool:
    """Check if an address is in GSU register space.

    Registers are at $3000-$32FF in banks $00-$3F and $80-$BF.

    Args:
        bank: Bank number
        addr: Address within bank

    Returns:
        True if address is in register space
    """
    # Check address range (bank doesn't matter much for registers)
    return REGISTER_START <= addr <= REGISTER_END


# =============================================================================
# Screen Buffer Calculations
# =============================================================================

# Valid screen configurations
VALID_HEIGHTS = {128, 160, 192}
VALID_COLORS = {4, 16, 256}

# Bits per pixel for each color mode
BPP_FOR_COLORS = {
    4: 2,    # 2bpp
    16: 4,   # 4bpp
    256: 8,  # 8bpp
}


def get_screen_buffer_size(height: int, colors: int) -> int:
    """Calculate screen buffer size in bytes.

    The screen is always 256 pixels wide.

    Args:
        height: Screen height in pixels (128, 160, or 192)
        colors: Number of colors (4, 16, or 256)

    Returns:
        Screen buffer size in bytes

    Raises:
        ValueError: If height or colors is invalid
    """
    if height not in VALID_HEIGHTS:
        raise ValueError(f"Invalid screen height: {height}. Must be 128, 160, or 192.")

    if colors not in VALID_COLORS:
        raise ValueError(f"Invalid color count: {colors}. Must be 4, 16, or 256.")

    width = 256
    bpp = BPP_FOR_COLORS[colors]

    # Size in bytes = width * height * bpp / 8
    return (width * height * bpp) // 8


def get_screen_base_address(scbr: int) -> int:
    """Calculate screen base address from SCBR register value.

    The SCBR register contains the high byte of the screen base address
    within GSU RAM. The low byte is always $00.

    Args:
        scbr: Value of SCBR register (0-255)

    Returns:
        Screen base address within RAM bank
    """
    return (scbr & 0xFF) << 8
