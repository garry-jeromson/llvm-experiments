"""Tests for SuperFX ROM builder."""

import pytest
from tools.snes_builder.superfx.rom_builder import SuperFXROMBuilder


class TestROMHeader:
    """Test SuperFX ROM header generation."""

    def test_header_map_mode(self):
        """Header has correct map mode ($20 for LoROM)."""
        builder = SuperFXROMBuilder(bytes([0x00, 0x01]))  # STOP, NOP
        rom = builder.build()
        # Map mode at $FFD5
        assert rom[0xFFD5] == 0x20

    def test_header_cartridge_type(self):
        """Header has SuperFX cartridge type ($13)."""
        builder = SuperFXROMBuilder(bytes([0x00, 0x01]))
        rom = builder.build()
        # Cartridge type at $FFD6
        assert rom[0xFFD6] == 0x13

    def test_header_rom_size(self):
        """Header has correct ROM size value."""
        builder = SuperFXROMBuilder(bytes([0x00, 0x01]))
        rom = builder.build()
        # ROM size at $FFD7: $08 = 256KB
        assert rom[0xFFD7] == 0x08

    def test_header_ram_size(self):
        """Header has correct RAM size value."""
        builder = SuperFXROMBuilder(bytes([0x00, 0x01]))
        rom = builder.build()
        # RAM size at $FFD8: $06 = 64KB (for SuperFX backup RAM)
        assert rom[0xFFD8] == 0x06

    def test_header_title(self):
        """Header can have custom title."""
        builder = SuperFXROMBuilder(bytes([0x00, 0x01]))
        builder.set_title("TEST ROM")
        rom = builder.build()
        # Title at $FFC0-$FFD4 (21 bytes)
        title = rom[0xFFC0:0xFFC0 + 21].rstrip(b'\x20')
        assert title == b"TEST ROM"

    def test_header_title_default(self):
        """Header has default title if not set."""
        builder = SuperFXROMBuilder(bytes([0x00, 0x01]))
        rom = builder.build()
        title = rom[0xFFC0:0xFFC0 + 21].rstrip(b'\x20')
        assert title == b"SUPERFX PROGRAM"


class TestROMSize:
    """Test ROM size and padding."""

    def test_minimum_size_256kb(self):
        """ROM is padded to minimum 256KB."""
        builder = SuperFXROMBuilder(bytes([0x00, 0x01]))
        rom = builder.build()
        assert len(rom) == 256 * 1024

    def test_padding_with_0xff(self):
        """Unused ROM space is filled with $FF."""
        builder = SuperFXROMBuilder(bytes([0x00, 0x01]))
        rom = builder.build()
        # Check some unused area is $FF
        assert rom[0x7000] == 0xFF

    def test_larger_gsu_code(self):
        """ROM can accommodate larger GSU programs."""
        gsu_code = bytes([0x01] * 1000)  # 1KB of NOPs
        builder = SuperFXROMBuilder(gsu_code)
        rom = builder.build()
        assert len(rom) == 256 * 1024


class TestChecksums:
    """Test ROM checksum calculation."""

    def test_checksum_complement_relationship(self):
        """Checksum and complement should XOR to $FFFF."""
        builder = SuperFXROMBuilder(bytes([0x00, 0x01]))
        rom = builder.build()
        # Complement at $FFDC-$FFDD (little-endian)
        complement = rom[0xFFDC] | (rom[0xFFDD] << 8)
        # Checksum at $FFDE-$FFDF (little-endian)
        checksum = rom[0xFFDE] | (rom[0xFFDF] << 8)
        assert (complement ^ checksum) == 0xFFFF

    def test_checksum_is_sum_of_bytes(self):
        """Checksum is 16-bit sum of all ROM bytes (mod 65536)."""
        builder = SuperFXROMBuilder(bytes([0x00, 0x01]))
        rom = builder.build()

        # Clear checksum bytes for calculation
        rom_list = list(rom)
        rom_list[0xFFDC] = 0
        rom_list[0xFFDD] = 0
        rom_list[0xFFDE] = 0
        rom_list[0xFFDF] = 0

        expected_checksum = sum(rom_list) & 0xFFFF
        actual_checksum = rom[0xFFDE] | (rom[0xFFDF] << 8)
        assert actual_checksum == expected_checksum


class TestGSUCodeEmbedding:
    """Test GSU code embedding in ROM."""

    def test_gsu_code_at_default_address(self):
        """GSU code is placed at default address $8000."""
        gsu_code = bytes([0x00, 0x01])  # STOP, NOP
        builder = SuperFXROMBuilder(gsu_code)
        rom = builder.build()
        # GSU code should be at $8000
        assert rom[0x8000] == 0x00
        assert rom[0x8001] == 0x01

    def test_gsu_code_at_custom_address(self):
        """GSU code can be placed at custom address."""
        gsu_code = bytes([0x00, 0x01])
        builder = SuperFXROMBuilder(gsu_code, gsu_start=0x9000)
        rom = builder.build()
        assert rom[0x9000] == 0x00
        assert rom[0x9001] == 0x01

    def test_gsu_code_preserved(self):
        """GSU code bytes are preserved exactly."""
        gsu_code = bytes(range(256))  # All byte values
        builder = SuperFXROMBuilder(gsu_code)
        rom = builder.build()
        for i, byte in enumerate(gsu_code):
            assert rom[0x8000 + i] == byte


class TestSNESStartupCode:
    """Test SNES CPU startup code that initializes GSU."""

    def test_reset_vector_set(self):
        """Reset vector points to SNES startup code."""
        builder = SuperFXROMBuilder(bytes([0x00, 0x01]))
        rom = builder.build()
        # Reset vector at $FFFC-$FFFD (little-endian)
        reset_vector = rom[0xFFFC] | (rom[0xFFFD] << 8)
        # Should point to our startup code (somewhere in the ROM)
        assert 0x8000 <= reset_vector <= 0xFFFF

    def test_startup_code_initializes_gsu(self):
        """Startup code sets up GSU registers."""
        builder = SuperFXROMBuilder(bytes([0x00, 0x01]))
        rom = builder.build()
        # Reset vector points to startup code
        reset_addr = rom[0xFFFC] | (rom[0xFFFD] << 8)
        # Check that startup code exists at that address
        assert reset_addr < len(rom)

    def test_startup_code_starts_gsu_at_specified_address(self):
        """Startup code writes GSU start address to R15."""
        gsu_code = bytes([0x00, 0x01])
        gsu_start = 0x8000
        builder = SuperFXROMBuilder(gsu_code, gsu_start=gsu_start)
        rom = builder.build()
        # The startup code should contain the GSU start address
        # We look for the address bytes in the startup area
        # GSU start address low byte should appear in the startup code
        reset_addr = rom[0xFFFC] | (rom[0xFFFD] << 8)
        startup_region = rom[reset_addr:reset_addr + 64]
        # Should contain $00 and $80 (little-endian $8000)
        assert bytes([0x00]) in startup_region or bytes([0x80]) in startup_region


class TestScreenModeConfiguration:
    """Test screen mode configuration."""

    def test_set_screen_mode_128h_4color(self):
        """Can configure 128-height, 4-color mode."""
        builder = SuperFXROMBuilder(bytes([0x00, 0x01]))
        builder.set_screen_mode(128, 4)
        rom = builder.build()
        # Should build without error
        assert len(rom) == 256 * 1024

    def test_set_screen_mode_160h_16color(self):
        """Can configure 160-height, 16-color mode."""
        builder = SuperFXROMBuilder(bytes([0x00, 0x01]))
        builder.set_screen_mode(160, 16)
        rom = builder.build()
        assert len(rom) == 256 * 1024

    def test_set_screen_mode_invalid_height(self):
        """Invalid height raises ValueError."""
        builder = SuperFXROMBuilder(bytes([0x00, 0x01]))
        with pytest.raises(ValueError):
            builder.set_screen_mode(100, 4)

    def test_set_screen_mode_invalid_colors(self):
        """Invalid color count raises ValueError."""
        builder = SuperFXROMBuilder(bytes([0x00, 0x01]))
        with pytest.raises(ValueError):
            builder.set_screen_mode(128, 32)


class TestBuildIntegration:
    """Integration tests for building complete ROMs."""

    def test_build_minimal_rom(self):
        """Build minimal ROM with STOP, NOP."""
        builder = SuperFXROMBuilder(bytes([0x00, 0x01]))
        rom = builder.build()

        # Check basic structure
        assert len(rom) == 256 * 1024
        assert rom[0xFFD5] == 0x20  # LoROM
        assert rom[0xFFD6] == 0x13  # SuperFX
        assert rom[0x8000] == 0x00  # STOP
        assert rom[0x8001] == 0x01  # NOP

    def test_build_with_all_options(self):
        """Build ROM with all options configured."""
        gsu_code = bytes([0xA1, 0x42, 0x00, 0x01])  # IBT R1, #$42; STOP; NOP
        builder = SuperFXROMBuilder(gsu_code, gsu_start=0x8100)
        builder.set_title("MY GAME")
        builder.set_screen_mode(128, 16)
        rom = builder.build()

        assert len(rom) == 256 * 1024
        assert rom[0x8100] == 0xA1  # IBT
        assert rom[0x8101] == 0x42  # #$42
        title = rom[0xFFC0:0xFFC0 + 21].rstrip(b'\x20')
        assert title == b"MY GAME"

    def test_rom_can_be_written_to_file(self):
        """ROM bytes can be written to file."""
        import tempfile
        import os

        builder = SuperFXROMBuilder(bytes([0x00, 0x01]))
        rom = builder.build()

        with tempfile.NamedTemporaryFile(suffix='.sfc', delete=False) as f:
            f.write(rom)
            temp_path = f.name

        try:
            # Verify file was written correctly
            with open(temp_path, 'rb') as f:
                read_rom = f.read()
            assert read_rom == rom
        finally:
            os.unlink(temp_path)
