#!/usr/bin/env python3
"""
Unit tests for font2tiles.py
"""

import sys
import unittest
from pathlib import Path

# Add parent directory to path
sys.path.insert(0, str(Path(__file__).parent.parent))

# Import module under test
from font2tiles import (
    rgb_to_bgr555,
    get_pixel_value,
    extract_char_2bpp,
    extract_char_4bpp,
    convert_font,
    format_assembly,
)

try:
    from PIL import Image
    HAS_PIL = True
except ImportError:
    HAS_PIL = False


class TestColorConversion(unittest.TestCase):
    """Test BGR555 color conversion (same as img2tiles)."""

    def test_black(self):
        self.assertEqual(rgb_to_bgr555(0, 0, 0), 0x0000)

    def test_white(self):
        self.assertEqual(rgb_to_bgr555(255, 255, 255), 0x7FFF)


@unittest.skipUnless(HAS_PIL, "PIL not available")
class TestPixelValue(unittest.TestCase):
    """Test pixel value extraction."""

    def test_indexed_image(self):
        """Get pixel value from indexed image."""
        img = Image.new('P', (8, 8))
        img.putpalette([0, 0, 0, 255, 255, 255] + [0] * (254 * 3))
        img.putpixel((0, 0), 1)

        self.assertEqual(get_pixel_value(img, 0, 0), 1)
        self.assertEqual(get_pixel_value(img, 1, 0), 0)

    def test_grayscale_image(self):
        """Get pixel value from grayscale image."""
        img = Image.new('L', (8, 8), 0)
        img.putpixel((0, 0), 255)  # White
        img.putpixel((1, 0), 128)  # Mid-gray (should be 1)
        img.putpixel((2, 0), 64)   # Dark gray (should be 0)

        self.assertEqual(get_pixel_value(img, 0, 0), 1)
        self.assertEqual(get_pixel_value(img, 1, 0), 1)  # >= 128
        self.assertEqual(get_pixel_value(img, 2, 0), 0)  # < 128

    def test_bitmap_image(self):
        """Get pixel value from 1-bit image."""
        img = Image.new('1', (8, 8), 0)
        img.putpixel((0, 0), 1)

        self.assertEqual(get_pixel_value(img, 0, 0), 1)
        self.assertEqual(get_pixel_value(img, 1, 0), 0)


@unittest.skipUnless(HAS_PIL, "PIL not available")
class TestCharExtraction(unittest.TestCase):
    """Test character glyph extraction."""

    def test_2bpp_empty_char(self):
        """Extract empty character in 2bpp."""
        img = Image.new('P', (8, 8), 0)
        img.putpalette([0, 0, 0] + [255] * (255 * 3))

        char_data = extract_char_2bpp(img, 0, 0, 8, 8)
        self.assertEqual(len(char_data), 16)
        self.assertTrue(all(b == 0 for b in char_data))

    def test_2bpp_solid_char(self):
        """Extract solid white character in 2bpp."""
        img = Image.new('P', (8, 8), 1)
        img.putpalette([0, 0, 0, 255, 255, 255] + [0] * (254 * 3))

        char_data = extract_char_2bpp(img, 0, 0, 8, 8)
        self.assertEqual(len(char_data), 16)
        # Color 1 means bitplane 0 is all 1s, bitplane 1 is all 0s
        for i in range(0, 16, 2):
            self.assertEqual(char_data[i], 0xFF)
            self.assertEqual(char_data[i + 1], 0x00)

    def test_4bpp_char_size(self):
        """4bpp character should be 32 bytes."""
        img = Image.new('P', (8, 8), 0)
        img.putpalette([0] * (256 * 3))

        char_data = extract_char_4bpp(img, 0, 0, 8, 8)
        self.assertEqual(len(char_data), 32)

    def test_glyph_offset(self):
        """Extract character at specific grid position."""
        # Create 16x8 image (2 characters wide)
        img = Image.new('P', (16, 8), 0)
        img.putpalette([0, 0, 0, 255, 255, 255] + [0] * (254 * 3))
        # Put something in second character position
        img.putpixel((8, 0), 1)  # First pixel of second char

        char0 = extract_char_2bpp(img, 0, 0, 8, 8)
        char1 = extract_char_2bpp(img, 1, 0, 8, 8)

        # First char should be empty
        self.assertTrue(all(b == 0 for b in char0))
        # Second char should have first pixel set
        self.assertEqual(char1[0] & 0x80, 0x80)  # High bit of first byte


@unittest.skipUnless(HAS_PIL, "PIL not available")
class TestFontConversion(unittest.TestCase):
    """Test full font conversion."""

    def test_convert_single_row(self):
        """Convert a single row of characters."""
        # 8 characters * 8 pixels = 64 pixels wide
        img = Image.new('P', (64, 8), 0)
        img.putpalette([0, 0, 0, 255, 255, 255] + [0] * (254 * 3))

        tiles, num_chars = convert_font(img, 8, 8, 8, 32, 2)

        self.assertEqual(len(tiles), 95)  # Full ASCII printable range
        self.assertEqual(num_chars, 95)
        # First 8 tiles from image, rest are blank padding
        for tile in tiles[8:]:
            self.assertTrue(all(b == 0 for b in tile))

    def test_auto_detect_columns(self):
        """Auto-detect number of columns."""
        img = Image.new('P', (80, 8), 0)  # 10 chars wide
        img.putpalette([0] * (256 * 3))

        tiles, num_chars = convert_font(img, 8, 8, 0, 32, 2)

        # Should still output 95 tiles (full ASCII range)
        self.assertEqual(len(tiles), 95)

    def test_4bpp_output(self):
        """Convert font in 4bpp mode."""
        img = Image.new('P', (64, 8), 0)
        img.putpalette([0] * (256 * 3))

        tiles, num_chars = convert_font(img, 8, 8, 8, 32, 4)

        # Each tile should be 32 bytes in 4bpp
        for tile in tiles:
            self.assertEqual(len(tile), 32)


class TestAssemblyOutput(unittest.TestCase):
    """Test assembly file generation."""

    def test_basic_output(self):
        """Generate basic assembly output."""
        tiles = [bytes(16)] * 95  # 95 empty 2bpp tiles

        output = format_assembly(tiles, "font", 2, 32)

        self.assertIn(".segment \"RODATA\"", output)
        self.assertIn(".global font_tiles", output)
        self.assertIn("font_num_chars = 95", output)
        self.assertIn("font_start_char = 32", output)
        self.assertIn("font_bpp = 2", output)

    def test_char_comments(self):
        """Assembly should have character comments."""
        tiles = [bytes(16)] * 95

        output = format_assembly(tiles, "font", 2, 32)

        self.assertIn("; Char 32 ' '", output)  # Space
        self.assertIn("; Char 65 'A'", output)  # Letter A
        self.assertIn("; Char 48 '0'", output)  # Digit 0

    def test_4bpp_output(self):
        """4bpp font should have correct bpp in output."""
        tiles = [bytes(32)] * 95  # 95 empty 4bpp tiles

        output = format_assembly(tiles, "font4", 4, 32)

        self.assertIn("font4_bpp = 4", output)


if __name__ == '__main__':
    unittest.main()
