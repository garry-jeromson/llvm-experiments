"""SuperFX ROM builder.

This module provides functionality to build SuperFX cartridge ROMs
with embedded GSU programs.
"""

from typing import Optional

from tools.snes_builder.superfx.gsu_registers import (
    SFR, PBR, SCBR, SCMR,
    SCMR_HT_128, SCMR_HT_160, SCMR_HT_192,
    SCMR_MD_4COLOR, SCMR_MD_16COLOR, SCMR_MD_256COLOR,
    SCMR_RON, SCMR_RAN,
    R15,
)
from tools.snes_builder.rom_utils import (
    CartType,
    MapMode,
    ROMSize,
    RAMSize,
    calculate_checksum,
    calculate_checksum_complement,
    create_header,
    write_checksum,
    HIROM_HEADER_OFFSET,
    MIN_SUPERFX_ROM_SIZE,
    HEADER_RESET_VECTOR_OFFSET,
)


class SuperFXROMBuilder:
    """Builder for SuperFX cartridge ROMs.

    Creates a valid SuperFX ROM image with:
    - Proper SNES header (LoROM mode, SuperFX type)
    - SNES startup code to initialize and start the GSU
    - Embedded GSU program at specified address
    - Correct checksums
    """

    # Valid screen configurations
    VALID_HEIGHTS = {128, 160, 192}
    VALID_COLORS = {4, 16, 256}

    def __init__(self, gsu_code: bytes, gsu_start: int = 0x8000):
        """Initialize ROM builder.

        Args:
            gsu_code: Assembled GSU program bytes
            gsu_start: Address where GSU code will be placed (default $8000)
        """
        self.gsu_code = gsu_code
        self.gsu_start = gsu_start
        self.title = "SUPERFX PROGRAM"
        self.screen_height = 128
        self.screen_colors = 4

    def set_title(self, title: str) -> None:
        """Set the ROM title (max 21 characters).

        Args:
            title: ROM title string
        """
        self.title = title[:21]

    def set_screen_mode(self, height: int, colors: int) -> None:
        """Set screen mode configuration.

        Args:
            height: Screen height (128, 160, or 192)
            colors: Number of colors (4, 16, or 256)

        Raises:
            ValueError: If height or colors is invalid
        """
        if height not in self.VALID_HEIGHTS:
            raise ValueError(f"Invalid height: {height}. Must be 128, 160, or 192.")
        if colors not in self.VALID_COLORS:
            raise ValueError(f"Invalid colors: {colors}. Must be 4, 16, or 256.")

        self.screen_height = height
        self.screen_colors = colors

    def build(self) -> bytes:
        """Build the complete ROM image.

        Returns:
            Complete ROM image as bytes
        """
        # Start with $FF-filled ROM
        rom = bytearray([0xFF] * MIN_SUPERFX_ROM_SIZE)

        # Embed GSU code
        self._embed_gsu_code(rom)

        # Create and embed SNES startup code
        startup_addr = self._embed_startup_code(rom)

        # Write header using shared utilities
        header = create_header(
            title=self.title,
            map_mode=MapMode.LOROM,
            cart_type=CartType.SUPERFX,
            rom_size=ROMSize.SIZE_256KB,
            ram_size=RAMSize.SIZE_64KB,
            reset_vector=startup_addr,
        )
        # Write header at HiROM location (file offset $FFC0 for 256KB+ ROMs)
        for i, byte in enumerate(header):
            rom[HIROM_HEADER_OFFSET + i] = byte

        # Calculate and write checksums using shared utility
        write_checksum(rom, HIROM_HEADER_OFFSET)

        return bytes(rom)

    def _embed_gsu_code(self, rom: bytearray) -> None:
        """Embed GSU code into ROM at specified address."""
        start = self.gsu_start
        for i, byte in enumerate(self.gsu_code):
            rom[start + i] = byte

    def _embed_startup_code(self, rom: bytearray) -> int:
        """Create SNES startup code that initializes GSU.

        Returns:
            Address of startup code
        """
        # Place startup code just before the vectors
        # We'll use $FF00 as a safe location
        startup_addr = 0xFF00

        # Build startup code
        startup = self._build_startup_code()

        # Write to ROM
        for i, byte in enumerate(startup):
            rom[startup_addr + i] = byte

        return startup_addr

    def _build_startup_code(self) -> bytes:
        """Build SNES startup code that initializes and starts the GSU.

        The startup code:
        1. Switches to native mode
        2. Sets up GSU registers
        3. Writes R15 to start GSU execution
        4. Waits for GSU to complete
        5. Loops forever
        """
        # Calculate SCMR value
        scmr_value = self._get_scmr_value()

        code = bytearray()

        # SEI - Disable interrupts
        code.append(0x78)

        # CLC - Clear carry
        code.append(0x18)

        # XCE - Exchange carry and emulation (switch to native mode)
        code.append(0xFB)

        # REP #$30 - 16-bit A and X/Y
        code.extend([0xC2, 0x30])

        # LDA #$0000 - Clear A
        code.extend([0xA9, 0x00, 0x00])

        # STA $3034 - PBR = 0 (program bank)
        code.extend([0x8D, 0x34, 0x30])

        # LDA #<scmr_value> - Screen mode
        code.extend([0xA9, scmr_value | SCMR_RON | SCMR_RAN, 0x00])

        # STA $303A - Write SCMR (give GSU bus control)
        code.extend([0x8D, 0x3A, 0x30])

        # LDA #<gsu_start_hi> - High byte of GSU start address
        gsu_hi = (self.gsu_start >> 8) & 0xFF
        code.extend([0xA9, gsu_hi, 0x00])

        # STA $301F - R15 high byte
        code.extend([0x8D, 0x1F, 0x30])

        # LDA #<gsu_start_lo> - Low byte of GSU start address
        gsu_lo = self.gsu_start & 0xFF
        code.extend([0xA9, gsu_lo, 0x00])

        # STA $301E - R15 low byte (this starts GSU!)
        code.extend([0x8D, 0x1E, 0x30])

        # Wait loop: check if GSU is still running
        wait_loop = len(code)

        # LDA $3030 - Read SFR low byte
        code.extend([0xAD, 0x30, 0x30])

        # AND #$0020 - Check G flag
        code.extend([0x29, 0x20, 0x00])

        # BNE wait_loop - Loop while GSU running
        # Offset = wait_loop - (current + 2)
        branch_offset = wait_loop - (len(code) + 2)
        code.extend([0xD0, branch_offset & 0xFF])

        # GSU finished - give bus back to SNES
        # LDA #$00
        code.extend([0xA9, 0x00, 0x00])

        # STA $303A - Clear SCMR (SNES owns buses)
        code.extend([0x8D, 0x3A, 0x30])

        # Infinite loop
        forever_loop = len(code)

        # BRA forever_loop
        code.extend([0x80, 0xFE])  # Branch to self

        return bytes(code)

    def _get_scmr_value(self) -> int:
        """Calculate SCMR register value from screen settings."""
        scmr = 0

        # Height bits
        if self.screen_height == 128:
            scmr |= SCMR_HT_128
        elif self.screen_height == 160:
            scmr |= SCMR_HT_160
        elif self.screen_height == 192:
            scmr |= SCMR_HT_192

        # Color depth bits
        if self.screen_colors == 4:
            scmr |= SCMR_MD_4COLOR
        elif self.screen_colors == 16:
            scmr |= SCMR_MD_16COLOR
        elif self.screen_colors == 256:
            scmr |= SCMR_MD_256COLOR

        return scmr
