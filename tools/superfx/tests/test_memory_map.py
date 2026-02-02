"""Tests for GSU memory map utilities."""

import pytest
from tools.superfx.memory_map import (
    # Address translation
    snes_to_gsu_rom,
    snes_to_gsu_ram,
    gsu_to_snes_rom,
    gsu_to_snes_ram,
    # Memory region detection
    is_rom_address,
    is_ram_address,
    is_register_address,
    # Screen buffer calculations
    get_screen_buffer_size,
    get_screen_base_address,
    # Memory region constants
    ROM_START, ROM_END,
    RAM_START, RAM_END,
    REGISTER_START, REGISTER_END,
)


class TestAddressTranslation:
    """Test SNES <-> GSU address translation."""

    def test_snes_to_gsu_rom_lorom_bank0(self):
        """SNES LoROM bank 0 $8000 -> GSU bank 0 $8000."""
        assert snes_to_gsu_rom(0x00, 0x8000) == (0x00, 0x8000)

    def test_snes_to_gsu_rom_lorom_bank1(self):
        """SNES LoROM bank 1 $8000 -> GSU bank 1 $8000."""
        assert snes_to_gsu_rom(0x01, 0x8000) == (0x01, 0x8000)

    def test_snes_to_gsu_rom_hirom_bank40(self):
        """SNES HiROM bank $40 $0000 -> GSU bank $40 $0000."""
        assert snes_to_gsu_rom(0x40, 0x0000) == (0x40, 0x0000)

    def test_snes_to_gsu_ram_bank70(self):
        """SNES RAM bank $70 -> GSU RAM bank 0."""
        assert snes_to_gsu_ram(0x70, 0x1234) == (0x70, 0x1234)

    def test_snes_to_gsu_ram_bank71(self):
        """SNES RAM bank $71 -> GSU RAM bank 1."""
        assert snes_to_gsu_ram(0x71, 0x5678) == (0x71, 0x5678)

    def test_gsu_to_snes_rom_lorom(self):
        """GSU ROM bank 0 -> SNES LoROM format."""
        bank, addr = gsu_to_snes_rom(0x00, 0x8000)
        assert bank == 0x00
        assert addr == 0x8000

    def test_gsu_to_snes_rom_hirom(self):
        """GSU ROM bank $40 -> SNES HiROM format."""
        bank, addr = gsu_to_snes_rom(0x40, 0x1234)
        assert bank == 0x40
        assert addr == 0x1234

    def test_gsu_to_snes_ram(self):
        """GSU RAM -> SNES format."""
        bank, addr = gsu_to_snes_ram(0x70, 0x1234)
        assert bank == 0x70
        assert addr == 0x1234


class TestMemoryRegionDetection:
    """Test memory region detection functions."""

    def test_is_rom_address_lorom(self):
        """LoROM addresses are ROM."""
        assert is_rom_address(0x00, 0x8000) == True
        assert is_rom_address(0x00, 0xFFFF) == True
        assert is_rom_address(0x3F, 0x8000) == True

    def test_is_rom_address_hirom(self):
        """HiROM addresses are ROM."""
        assert is_rom_address(0x40, 0x0000) == True
        assert is_rom_address(0x5F, 0xFFFF) == True

    def test_is_rom_address_low_area(self):
        """Low area of bank is not ROM (for LoROM)."""
        assert is_rom_address(0x00, 0x7FFF) == False

    def test_is_ram_address_bank70(self):
        """Bank $70 is RAM."""
        assert is_ram_address(0x70, 0x0000) == True
        assert is_ram_address(0x70, 0xFFFF) == True

    def test_is_ram_address_bank71(self):
        """Bank $71 is RAM."""
        assert is_ram_address(0x71, 0x0000) == True

    def test_is_ram_address_rom_bank(self):
        """ROM banks are not RAM."""
        assert is_ram_address(0x00, 0x8000) == False
        assert is_ram_address(0x40, 0x0000) == False

    def test_is_register_address(self):
        """GSU registers at $3000-$32FF."""
        assert is_register_address(0x00, 0x3000) == True
        assert is_register_address(0x00, 0x32FF) == True
        assert is_register_address(0x00, 0x2FFF) == False
        assert is_register_address(0x00, 0x3300) == False


class TestScreenBufferCalculations:
    """Test screen buffer size calculations.

    Note: These sizes are based on 256-pixel width screens with the
    specified heights and color depths. The size is calculated as:
    width * height * bpp / 8 bytes.
    """

    def test_screen_buffer_128h_4color(self):
        """128-pixel height, 4-color mode (2bpp) = 8KB."""
        # 256 * 128 * 2 / 8 = 8192 bytes
        size = get_screen_buffer_size(128, 4)
        assert size == 8192

    def test_screen_buffer_128h_16color(self):
        """128-pixel height, 16-color mode (4bpp) = 16KB."""
        # 256 * 128 * 4 / 8 = 16384 bytes
        size = get_screen_buffer_size(128, 16)
        assert size == 16384

    def test_screen_buffer_128h_256color(self):
        """128-pixel height, 256-color mode (8bpp) = 32KB."""
        # 256 * 128 * 8 / 8 = 32768 bytes
        size = get_screen_buffer_size(128, 256)
        assert size == 32768

    def test_screen_buffer_160h_4color(self):
        """160-pixel height, 4-color mode (2bpp) = 10KB."""
        # 256 * 160 * 2 / 8 = 10240 bytes
        size = get_screen_buffer_size(160, 4)
        assert size == 10240

    def test_screen_buffer_192h_4color(self):
        """192-pixel height, 4-color mode (2bpp) = 12KB."""
        # 256 * 192 * 2 / 8 = 12288 bytes
        size = get_screen_buffer_size(192, 4)
        assert size == 12288

    def test_screen_buffer_invalid_height(self):
        """Invalid height raises ValueError."""
        with pytest.raises(ValueError):
            get_screen_buffer_size(100, 4)

    def test_screen_buffer_invalid_colors(self):
        """Invalid color count raises ValueError."""
        with pytest.raises(ValueError):
            get_screen_buffer_size(128, 32)

    def test_screen_base_address(self):
        """Calculate screen base address from SCBR value."""
        # SCBR value is the high byte of the screen base address in RAM
        # SCBR $00 -> base $000000
        # SCBR $10 -> base $001000
        assert get_screen_base_address(0x00) == 0x0000
        assert get_screen_base_address(0x10) == 0x1000
        assert get_screen_base_address(0x20) == 0x2000
        assert get_screen_base_address(0xFF) == 0xFF00


class TestMemoryRegionConstants:
    """Test memory region constants are correct."""

    def test_rom_region_constants(self):
        """ROM region constants."""
        assert ROM_START == 0x8000
        assert ROM_END == 0xFFFF

    def test_ram_region_constants(self):
        """RAM region constants (bank-relative)."""
        assert RAM_START == 0x0000
        assert RAM_END == 0xFFFF

    def test_register_region_constants(self):
        """Register region constants."""
        assert REGISTER_START == 0x3000
        assert REGISTER_END == 0x32FF
