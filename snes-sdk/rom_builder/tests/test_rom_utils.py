"""Tests for ROM utilities."""

import pytest
import tempfile
import os

from tools.snes_builder.rom_utils import (
    CartType,
    MapMode,
    ROMSize,
    RAMSize,
    calculate_checksum,
    calculate_checksum_complement,
    create_header,
    write_header,
    write_checksum,
    fix_checksum,
    pad_rom,
    get_rom_size_code,
    LOROM_HEADER_OFFSET,
    MIN_ROM_SIZE,
    MIN_SUPERFX_ROM_SIZE,
    HEADER_TITLE_OFFSET,
    HEADER_MAP_MODE_OFFSET,
    HEADER_TYPE_OFFSET,
    HEADER_ROM_SIZE_OFFSET,
    HEADER_RAM_SIZE_OFFSET,
    HEADER_CHECKSUM_COMP_OFFSET,
    HEADER_CHECKSUM_OFFSET,
    HEADER_RESET_VECTOR_OFFSET,
)


class TestCartTypeConstants:
    """Test cartridge type constants."""

    def test_rom_only_value(self):
        """ROM_ONLY has correct value."""
        assert CartType.ROM_ONLY == 0x00

    def test_superfx_value(self):
        """SUPERFX has correct value."""
        assert CartType.SUPERFX == 0x13

    def test_sa1_value(self):
        """SA1 has correct value."""
        assert CartType.SA1 == 0x34


class TestMapModeConstants:
    """Test map mode constants."""

    def test_lorom_value(self):
        """LOROM has correct value."""
        assert MapMode.LOROM == 0x20

    def test_hirom_value(self):
        """HIROM has correct value."""
        assert MapMode.HIROM == 0x21


class TestROMSizeConstants:
    """Test ROM size constants."""

    def test_256kb_value(self):
        """SIZE_256KB has correct value."""
        assert ROMSize.SIZE_256KB == 0x08

    def test_1mb_value(self):
        """SIZE_1MB has correct value."""
        assert ROMSize.SIZE_1MB == 0x0A


class TestCalculateChecksum:
    """Test checksum calculation."""

    def test_empty_data(self):
        """Checksum of empty data is 0."""
        assert calculate_checksum(b'', zero_checksum_area=False) == 0

    def test_simple_data(self):
        """Checksum is sum of bytes mod 65536."""
        data = bytes([0x01, 0x02, 0x03])
        assert calculate_checksum(data, zero_checksum_area=False) == 6

    def test_overflow_wraps(self):
        """Checksum wraps at 65536."""
        # 257 bytes of 0xFF: sum = 257 * 255 = 65535
        # Actually 257 bytes of 0xFF sum to 257 * 255 = 65535 which fits exactly
        # Let's use 258 bytes: 258 * 255 = 65790 = 254 mod 65536
        data = bytes([0xFF] * 258)
        assert calculate_checksum(data, zero_checksum_area=False) == (258 * 255) & 0xFFFF


class TestCalculateChecksumComplement:
    """Test checksum complement calculation."""

    def test_complement_xor(self):
        """Complement is XOR with 0xFFFF."""
        assert calculate_checksum_complement(0x0000) == 0xFFFF
        assert calculate_checksum_complement(0xFFFF) == 0x0000
        assert calculate_checksum_complement(0x1234) == 0xEDCB

    def test_complement_relationship(self):
        """Checksum XOR complement equals 0xFFFF."""
        for checksum in [0x0000, 0x1234, 0x8000, 0xFFFF]:
            complement = calculate_checksum_complement(checksum)
            assert (checksum ^ complement) == 0xFFFF


class TestCreateHeader:
    """Test header creation."""

    def test_header_size(self):
        """Header is 64 bytes."""
        header = create_header()
        assert len(header) == 64

    def test_default_title(self):
        """Default title is set."""
        header = create_header()
        title = header[HEADER_TITLE_OFFSET:HEADER_TITLE_OFFSET + 21].rstrip(b' ')
        assert title == b"SNES PROGRAM"

    def test_custom_title(self):
        """Custom title is set correctly."""
        header = create_header(title="MY GAME")
        title = header[HEADER_TITLE_OFFSET:HEADER_TITLE_OFFSET + 21].rstrip(b' ')
        assert title == b"MY GAME"

    def test_title_truncation(self):
        """Long titles are truncated to 21 characters."""
        header = create_header(title="THIS IS A VERY LONG TITLE THAT EXCEEDS 21 CHARS")
        title = header[HEADER_TITLE_OFFSET:HEADER_TITLE_OFFSET + 21]
        assert len(title) == 21

    def test_title_padding(self):
        """Short titles are padded with spaces."""
        header = create_header(title="HI")
        title = header[HEADER_TITLE_OFFSET:HEADER_TITLE_OFFSET + 21]
        assert title == b"HI" + b' ' * 19

    def test_lorom_map_mode(self):
        """LoROM map mode is set."""
        header = create_header(map_mode=MapMode.LOROM)
        assert header[HEADER_MAP_MODE_OFFSET] == 0x20

    def test_hirom_map_mode(self):
        """HiROM map mode is set."""
        header = create_header(map_mode=MapMode.HIROM)
        assert header[HEADER_MAP_MODE_OFFSET] == 0x21

    def test_cart_type_superfx(self):
        """SuperFX cart type is set."""
        header = create_header(cart_type=CartType.SUPERFX)
        assert header[HEADER_TYPE_OFFSET] == 0x13

    def test_rom_size(self):
        """ROM size is set."""
        header = create_header(rom_size=ROMSize.SIZE_256KB)
        assert header[HEADER_ROM_SIZE_OFFSET] == 0x08

    def test_ram_size(self):
        """RAM size is set."""
        header = create_header(ram_size=RAMSize.SIZE_64KB)
        assert header[HEADER_RAM_SIZE_OFFSET] == 0x06

    def test_reset_vector(self):
        """Reset vector is set (little-endian)."""
        header = create_header(reset_vector=0x8000)
        low = header[HEADER_RESET_VECTOR_OFFSET]
        high = header[HEADER_RESET_VECTOR_OFFSET + 1]
        assert (high << 8) | low == 0x8000

    def test_reset_vector_custom(self):
        """Custom reset vector is set."""
        header = create_header(reset_vector=0xFF00)
        low = header[HEADER_RESET_VECTOR_OFFSET]
        high = header[HEADER_RESET_VECTOR_OFFSET + 1]
        assert (high << 8) | low == 0xFF00


class TestWriteHeader:
    """Test header writing to ROM."""

    def test_write_at_lorom_offset(self):
        """Header written at LoROM offset."""
        rom = bytearray([0xFF] * (LOROM_HEADER_OFFSET + 64))
        header = create_header(title="TEST")
        write_header(rom, header, LOROM_HEADER_OFFSET)

        # Check title was written
        title = rom[LOROM_HEADER_OFFSET:LOROM_HEADER_OFFSET + 21].rstrip(b' ')
        assert title == b"TEST"


class TestWriteChecksum:
    """Test checksum writing to ROM."""

    def test_checksum_written(self):
        """Checksum and complement are written."""
        rom = bytearray([0xFF] * MIN_ROM_SIZE)
        checksum, complement = write_checksum(rom, LOROM_HEADER_OFFSET)

        # Read back values
        comp_off = LOROM_HEADER_OFFSET + HEADER_CHECKSUM_COMP_OFFSET
        check_off = LOROM_HEADER_OFFSET + HEADER_CHECKSUM_OFFSET

        read_complement = rom[comp_off] | (rom[comp_off + 1] << 8)
        read_checksum = rom[check_off] | (rom[check_off + 1] << 8)

        assert read_checksum == checksum
        assert read_complement == complement

    def test_checksum_complement_relationship(self):
        """Checksum XOR complement equals 0xFFFF."""
        rom = bytearray([0xFF] * MIN_ROM_SIZE)
        checksum, complement = write_checksum(rom, LOROM_HEADER_OFFSET)
        assert (checksum ^ complement) == 0xFFFF


class TestFixChecksum:
    """Test checksum fixing in ROM files."""

    def test_fix_checksum_file(self):
        """Fix checksum in ROM file."""
        # Create a minimal ROM
        rom = bytearray([0xFF] * MIN_ROM_SIZE)
        rom[LOROM_HEADER_OFFSET + HEADER_MAP_MODE_OFFSET] = MapMode.LOROM

        with tempfile.NamedTemporaryFile(suffix='.sfc', delete=False) as f:
            f.write(rom)
            temp_path = f.name

        try:
            checksum, complement = fix_checksum(temp_path)
            assert (checksum ^ complement) == 0xFFFF

            # Verify file was updated
            with open(temp_path, 'rb') as f:
                updated_rom = f.read()

            comp_off = LOROM_HEADER_OFFSET + HEADER_CHECKSUM_COMP_OFFSET
            check_off = LOROM_HEADER_OFFSET + HEADER_CHECKSUM_OFFSET

            read_complement = updated_rom[comp_off] | (updated_rom[comp_off + 1] << 8)
            read_checksum = updated_rom[check_off] | (updated_rom[check_off + 1] << 8)

            assert read_checksum == checksum
            assert read_complement == complement
        finally:
            os.unlink(temp_path)

    def test_fix_checksum_too_small(self):
        """Raise error for ROM smaller than minimum."""
        with tempfile.NamedTemporaryFile(suffix='.sfc', delete=False) as f:
            f.write(b'\x00' * 100)  # Too small
            temp_path = f.name

        try:
            with pytest.raises(ValueError):
                fix_checksum(temp_path)
        finally:
            os.unlink(temp_path)


class TestPadRom:
    """Test ROM padding."""

    def test_pad_to_size(self):
        """ROM is padded to minimum size."""
        rom = bytearray([0x00] * 100)
        pad_rom(rom, 256)
        assert len(rom) == 256

    def test_pad_with_ff(self):
        """ROM is padded with 0xFF by default."""
        rom = bytearray([0x00] * 100)
        pad_rom(rom, 256)
        assert rom[100] == 0xFF
        assert rom[255] == 0xFF

    def test_pad_with_custom_byte(self):
        """ROM can be padded with custom byte."""
        rom = bytearray([0x00] * 100)
        pad_rom(rom, 256, fill_byte=0xAA)
        assert rom[100] == 0xAA
        assert rom[255] == 0xAA

    def test_no_pad_if_large_enough(self):
        """ROM not padded if already large enough."""
        rom = bytearray([0x00] * 300)
        pad_rom(rom, 256)
        assert len(rom) == 300


class TestGetRomSizeCode:
    """Test ROM size code calculation."""

    def test_32kb(self):
        """32KB returns SIZE_32KB."""
        assert get_rom_size_code(32 * 1024) == ROMSize.SIZE_32KB

    def test_64kb(self):
        """64KB returns SIZE_64KB."""
        assert get_rom_size_code(64 * 1024) == ROMSize.SIZE_64KB

    def test_256kb(self):
        """256KB returns SIZE_256KB."""
        assert get_rom_size_code(256 * 1024) == ROMSize.SIZE_256KB

    def test_1mb(self):
        """1MB returns SIZE_1MB."""
        assert get_rom_size_code(1024 * 1024) == ROMSize.SIZE_1MB

    def test_smaller_rounds_up(self):
        """Smaller sizes round up to next power of 2."""
        assert get_rom_size_code(50 * 1024) == ROMSize.SIZE_64KB


class TestConstantValues:
    """Test constant values are correct."""

    def test_lorom_header_offset(self):
        """LoROM header offset is correct."""
        assert LOROM_HEADER_OFFSET == 0x7FC0

    def test_min_rom_size(self):
        """Minimum ROM size is 32KB."""
        assert MIN_ROM_SIZE == 32 * 1024

    def test_min_superfx_size(self):
        """Minimum SuperFX ROM size is 256KB."""
        assert MIN_SUPERFX_ROM_SIZE == 256 * 1024
