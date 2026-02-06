#!/usr/bin/env python3
"""
Unit tests for img2tiles.py
"""

import sys
import unittest
from pathlib import Path
from io import BytesIO

# Add parent directory to path
sys.path.insert(0, str(Path(__file__).parent.parent))

# Import module under test
from img2tiles import (
    rgb_to_bgr555,
    extract_palette,
    extract_tile_2bpp,
    extract_tile_4bpp,
    convert_image,
    format_assembly,
)

try:
    from PIL import Image
    HAS_PIL = True
except ImportError:
    HAS_PIL = False


class TestColorConversion(unittest.TestCase):
    """Test BGR555 color conversion."""

    def test_black(self):
        """Black should be 0."""
        self.assertEqual(rgb_to_bgr555(0, 0, 0), 0x0000)

    def test_white(self):
        """White should be 0x7FFF (all bits set)."""
        self.assertEqual(rgb_to_bgr555(255, 255, 255), 0x7FFF)

    def test_red(self):
        """Pure red (255, 0, 0) -> BGR555 red (31 in low bits)."""
        result = rgb_to_bgr555(255, 0, 0)
        self.assertEqual(result & 0x001F, 31)  # Red in bits 0-4
        self.assertEqual(result & 0x03E0, 0)   # Green in bits 5-9
        self.assertEqual(result & 0x7C00, 0)   # Blue in bits 10-14

    def test_green(self):
        """Pure green (0, 255, 0) -> BGR555 green."""
        result = rgb_to_bgr555(0, 255, 0)
        self.assertEqual(result & 0x001F, 0)
        self.assertEqual((result >> 5) & 0x1F, 31)
        self.assertEqual(result & 0x7C00, 0)

    def test_blue(self):
        """Pure blue (0, 0, 255) -> BGR555 blue."""
        result = rgb_to_bgr555(0, 0, 255)
        self.assertEqual(result & 0x001F, 0)
        self.assertEqual(result & 0x03E0, 0)
        self.assertEqual((result >> 10) & 0x1F, 31)

    def test_quantization(self):
        """Colors should be quantized to 5-bit (0-31)."""
        # 128 -> 128 >> 3 = 16
        result = rgb_to_bgr555(128, 0, 0)
        self.assertEqual(result & 0x1F, 16)


@unittest.skipUnless(HAS_PIL, "PIL not available")
class TestPaletteExtraction(unittest.TestCase):
    """Test palette extraction from images."""

    def test_indexed_image(self):
        """Extract palette from indexed image."""
        # Create a simple indexed image with specific palette
        img = Image.new('P', (8, 8))
        img.putpalette([0, 0, 0, 255, 0, 0] + [0] * (256 * 3 - 6))
        img.putpixel((0, 0), 1)  # Red pixel

        # Indexed images return up to max_colors from the stored palette
        palette = extract_palette(img, 4)
        self.assertEqual(len(palette), 4)  # Gets min(palette_size, max_colors)
        self.assertEqual(palette[0], 0x0000)  # Black
        self.assertEqual(palette[1], 0x001F)  # Red (BGR555)

    def test_rgb_image(self):
        """Extract palette from RGB image."""
        img = Image.new('RGB', (8, 8), (0, 0, 0))
        img.putpixel((0, 0), (255, 255, 255))
        img.putpixel((1, 0), (255, 0, 0))

        palette = extract_palette(img, 4)
        self.assertEqual(len(palette), 3)  # Black, white, red
        self.assertIn(0x0000, palette)  # Black
        self.assertIn(0x7FFF, palette)  # White


@unittest.skipUnless(HAS_PIL, "PIL not available")
class TestTileExtraction(unittest.TestCase):
    """Test tile data extraction."""

    def test_2bpp_empty_tile(self):
        """Extract empty (black) tile in 2bpp."""
        img = Image.new('P', (8, 8))
        img.putpalette([0, 0, 0] + [255] * (255 * 3))

        tile = extract_tile_2bpp(img, 0, 0, [0])
        self.assertEqual(len(tile), 16)  # 8 rows * 2 bytes
        self.assertTrue(all(b == 0 for b in tile))

    def test_2bpp_solid_tile(self):
        """Extract solid color 1 tile in 2bpp."""
        img = Image.new('P', (8, 8))
        img.putpalette([0, 0, 0, 255, 255, 255] + [0] * (254 * 3))
        # Fill with color 1
        for y in range(8):
            for x in range(8):
                img.putpixel((x, y), 1)

        tile = extract_tile_2bpp(img, 0, 0, [0, 0x7FFF])
        self.assertEqual(len(tile), 16)
        # Color 1 means bit 0 is set for all pixels
        for i in range(0, 16, 2):
            self.assertEqual(tile[i], 0xFF)  # Bitplane 0 all 1s
            self.assertEqual(tile[i + 1], 0x00)  # Bitplane 1 all 0s

    def test_4bpp_tile_size(self):
        """4bpp tiles should be 32 bytes."""
        img = Image.new('P', (8, 8))
        img.putpalette([0] * (256 * 3))

        tile = extract_tile_4bpp(img, 0, 0, [0])
        self.assertEqual(len(tile), 32)

    def test_2bpp_pattern(self):
        """Test specific bit pattern in 2bpp."""
        img = Image.new('P', (8, 8))
        img.putpalette([0, 0, 0, 255, 255, 255] + [0] * (254 * 3))
        # Set alternating pixels in first row
        for x in range(8):
            img.putpixel((x, 0), x % 2)

        tile = extract_tile_2bpp(img, 0, 0, [0, 0x7FFF])
        # First row: 0,1,0,1,0,1,0,1 -> bitplane 0 = 0b01010101 = 0x55
        self.assertEqual(tile[0], 0x55)
        self.assertEqual(tile[1], 0x00)


@unittest.skipUnless(HAS_PIL, "PIL not available")
class TestImageConversion(unittest.TestCase):
    """Test full image conversion."""

    def test_convert_small_image(self):
        """Convert 8x8 image to single tile."""
        img = Image.new('P', (8, 8))
        img.putpalette([0, 0, 0, 255, 255, 255] + [0] * (254 * 3))

        tiles, tilemap, palette = convert_image(img, 2, optimize=False)

        self.assertEqual(len(tiles), 1)
        self.assertIsNone(tilemap)
        # 2bpp mode extracts up to 4 colors from indexed palette
        self.assertEqual(len(palette), 4)

    def test_convert_with_optimization(self):
        """Convert with duplicate tile removal."""
        # 16x8 image = 2 tiles, but both are identical
        img = Image.new('P', (16, 8))
        img.putpalette([0, 0, 0] + [255] * (255 * 3))

        tiles, tilemap, palette = convert_image(img, 2, optimize=True)

        self.assertEqual(len(tiles), 1)  # Only 1 unique tile
        self.assertEqual(len(tilemap), 2)  # 2 entries in tilemap
        self.assertEqual(tilemap, [0, 0])  # Both point to same tile


class TestAssemblyOutput(unittest.TestCase):
    """Test assembly file generation."""

    def test_basic_output(self):
        """Generate basic assembly output."""
        tiles = [bytes(16)]  # One empty 2bpp tile
        palette = [0x0000, 0x7FFF]

        output = format_assembly(tiles, None, palette, "test", True, False, 0, 0)

        self.assertIn(".segment \"RODATA\"", output)
        self.assertIn(".global test_tiles", output)
        self.assertIn(".global test_palette", output)
        self.assertIn("test_tiles:", output)

    def test_tilemap_output(self):
        """Generate output with tilemap."""
        tiles = [bytes(16)]
        palette = [0x0000]
        tilemap = [0, 0, 0, 0]

        output = format_assembly(tiles, tilemap, palette, "gfx", True, True, 2, 2)

        self.assertIn(".global gfx_tilemap", output)
        self.assertIn("gfx_tilemap_width = 2", output)
        self.assertIn("gfx_tilemap_height = 2", output)

    def test_palette_size(self):
        """Palette size should be correct."""
        tiles = [bytes(16)]
        palette = [0x0000, 0x7FFF, 0x001F, 0x7C00]

        output = format_assembly(tiles, None, palette, "test", True, False, 0, 0)

        self.assertIn("test_palette_count = 4", output)


if __name__ == '__main__':
    unittest.main()
