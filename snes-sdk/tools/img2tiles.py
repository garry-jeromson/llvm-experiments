#!/usr/bin/env python3
"""
img2tiles.py - Convert PNG images to SNES tile data

Usage:
    python img2tiles.py input.png -o output.s [options]

Options:
    -o, --output      Output assembly file (default: stdout)
    --mode            Bits per pixel: 2, 4, or 8 (default: 4)
    --tilemap         Generate tilemap data
    --sprite-size     Sprite size for sprite sheets (8, 16, 32, 64)
    --optimize        Remove duplicate tiles
    --palette         Output palette data
    --palette-offset  Starting palette index (default: 0)
    --label           Label prefix for generated data
    --raw             Output raw binary instead of assembly
"""

import argparse
import sys
from pathlib import Path
from typing import List, Tuple, Optional

try:
    from PIL import Image
    HAS_PIL = True
except ImportError:
    HAS_PIL = False
    Image = None  # type: ignore


def rgb_to_bgr555(r: int, g: int, b: int) -> int:
    """Convert 8-bit RGB to BGR555 format."""
    r5 = r >> 3
    g5 = g >> 3
    b5 = b >> 3
    return (b5 << 10) | (g5 << 5) | r5


def extract_palette(img: Image.Image, max_colors: int = 256) -> List[int]:
    """Extract unique colors from image as BGR555 values."""
    if img.mode == 'P':
        # Indexed image - extract palette
        palette = img.getpalette()
        num_colors = min(len(palette) // 3, max_colors)
        colors = []
        for i in range(num_colors):
            r, g, b = palette[i*3:i*3+3]
            colors.append(rgb_to_bgr555(r, g, b))
        return colors

    # RGB image - find unique colors
    img_rgb = img.convert('RGB')
    width, height = img_rgb.size
    unique = []
    seen = set()

    for y in range(height):
        for x in range(width):
            r, g, b = img_rgb.getpixel((x, y))
            bgr555 = rgb_to_bgr555(r, g, b)
            if bgr555 not in seen:
                seen.add(bgr555)
                unique.append(bgr555)
                if len(unique) >= max_colors:
                    return unique

    return unique


def get_pixel_index(img: Image.Image, x: int, y: int, palette: List[int]) -> int:
    """Get palette index for pixel at (x, y)."""
    if img.mode == 'P':
        return img.getpixel((x, y))

    r, g, b = img.convert('RGB').getpixel((x, y))[:3]
    bgr555 = rgb_to_bgr555(r, g, b)

    try:
        return palette.index(bgr555)
    except ValueError:
        return 0


def extract_tile_2bpp(img: Image.Image, tx: int, ty: int, palette: List[int]) -> bytes:
    """Extract an 8x8 tile in 2bpp format (16 bytes)."""
    data = []

    for row in range(8):
        byte0 = 0  # Bit 0 of each pixel
        byte1 = 0  # Bit 1 of each pixel

        for col in range(8):
            px = get_pixel_index(img, tx * 8 + col, ty * 8 + row, palette) & 0x03

            if px & 0x01:
                byte0 |= (0x80 >> col)
            if px & 0x02:
                byte1 |= (0x80 >> col)

        data.append(byte0)
        data.append(byte1)

    return bytes(data)


def extract_tile_4bpp(img: Image.Image, tx: int, ty: int, palette: List[int]) -> bytes:
    """Extract an 8x8 tile in 4bpp format (32 bytes)."""
    data = []

    # Bitplanes 0-1 first
    for row in range(8):
        byte0 = 0
        byte1 = 0

        for col in range(8):
            px = get_pixel_index(img, tx * 8 + col, ty * 8 + row, palette) & 0x0F

            if px & 0x01:
                byte0 |= (0x80 >> col)
            if px & 0x02:
                byte1 |= (0x80 >> col)

        data.append(byte0)
        data.append(byte1)

    # Bitplanes 2-3
    for row in range(8):
        byte2 = 0
        byte3 = 0

        for col in range(8):
            px = get_pixel_index(img, tx * 8 + col, ty * 8 + row, palette) & 0x0F

            if px & 0x04:
                byte2 |= (0x80 >> col)
            if px & 0x08:
                byte3 |= (0x80 >> col)

        data.append(byte2)
        data.append(byte3)

    return bytes(data)


def extract_tile_8bpp(img: Image.Image, tx: int, ty: int, palette: List[int]) -> bytes:
    """Extract an 8x8 tile in 8bpp format (64 bytes)."""
    data = []

    # Bitplanes 0-1
    for row in range(8):
        byte0 = 0
        byte1 = 0
        for col in range(8):
            px = get_pixel_index(img, tx * 8 + col, ty * 8 + row, palette)
            if px & 0x01:
                byte0 |= (0x80 >> col)
            if px & 0x02:
                byte1 |= (0x80 >> col)
        data.append(byte0)
        data.append(byte1)

    # Bitplanes 2-3
    for row in range(8):
        byte2 = 0
        byte3 = 0
        for col in range(8):
            px = get_pixel_index(img, tx * 8 + col, ty * 8 + row, palette)
            if px & 0x04:
                byte2 |= (0x80 >> col)
            if px & 0x08:
                byte3 |= (0x80 >> col)
        data.append(byte2)
        data.append(byte3)

    # Bitplanes 4-5
    for row in range(8):
        byte4 = 0
        byte5 = 0
        for col in range(8):
            px = get_pixel_index(img, tx * 8 + col, ty * 8 + row, palette)
            if px & 0x10:
                byte4 |= (0x80 >> col)
            if px & 0x20:
                byte5 |= (0x80 >> col)
        data.append(byte4)
        data.append(byte5)

    # Bitplanes 6-7
    for row in range(8):
        byte6 = 0
        byte7 = 0
        for col in range(8):
            px = get_pixel_index(img, tx * 8 + col, ty * 8 + row, palette)
            if px & 0x40:
                byte6 |= (0x80 >> col)
            if px & 0x80:
                byte7 |= (0x80 >> col)
        data.append(byte6)
        data.append(byte7)

    return bytes(data)


def convert_image(img: Image.Image, bpp: int, optimize: bool = False) -> Tuple[List[bytes], List[int], Optional[List[int]]]:
    """
    Convert image to tile data.

    Returns:
        tiles: List of tile data (bytes)
        tilemap: List of tile indices (or None if optimize=False)
        palette: List of BGR555 colors
    """
    width, height = img.size
    tiles_x = width // 8
    tiles_y = height // 8

    # Get color count based on BPP
    max_colors = {2: 4, 4: 16, 8: 256}[bpp]
    palette = extract_palette(img, max_colors)

    # Extract tiles
    extract_fn = {2: extract_tile_2bpp, 4: extract_tile_4bpp, 8: extract_tile_8bpp}[bpp]

    all_tiles = []
    tilemap = []

    for ty in range(tiles_y):
        for tx in range(tiles_x):
            tile_data = extract_fn(img, tx, ty, palette)

            if optimize:
                # Check for duplicate
                try:
                    idx = all_tiles.index(tile_data)
                except ValueError:
                    idx = len(all_tiles)
                    all_tiles.append(tile_data)
                tilemap.append(idx)
            else:
                all_tiles.append(tile_data)
                tilemap.append(len(all_tiles) - 1)

    return all_tiles, tilemap if optimize else None, palette


def format_assembly(tiles: List[bytes], tilemap: Optional[List[int]], palette: List[int],
                    label: str, include_palette: bool, include_tilemap: bool,
                    tiles_x: int = 0, tiles_y: int = 0) -> str:
    """Format data as ca65 assembly."""
    lines = [
        f"; Generated by img2tiles.py",
        f"; Tiles: {len(tiles)}, Colors: {len(palette)}",
        "",
        ".segment \"RODATA\"",
        "",
    ]

    # Tile data
    lines.append(f".global {label}_tiles")
    lines.append(f"{label}_tiles:")

    for i, tile in enumerate(tiles):
        lines.append(f"    ; Tile {i}")
        for j in range(0, len(tile), 16):
            chunk = tile[j:j+16]
            hex_str = ", ".join(f"${b:02X}" for b in chunk)
            lines.append(f"    .byte {hex_str}")

    lines.append(f"{label}_tiles_end:")
    lines.append(f".global {label}_tiles_size")
    lines.append(f"{label}_tiles_size = {label}_tiles_end - {label}_tiles")
    lines.append("")

    # Tilemap
    if include_tilemap and tilemap is not None:
        lines.append(f".global {label}_tilemap")
        lines.append(f"{label}_tilemap:")

        for i in range(0, len(tilemap), 16):
            chunk = tilemap[i:i+16]
            hex_str = ", ".join(f"${t:04X}" for t in chunk)
            lines.append(f"    .word {hex_str}")

        lines.append(f"{label}_tilemap_end:")
        lines.append(f".global {label}_tilemap_size")
        lines.append(f"{label}_tilemap_size = {label}_tilemap_end - {label}_tilemap")
        lines.append(f".global {label}_tilemap_width")
        lines.append(f"{label}_tilemap_width = {tiles_x}")
        lines.append(f".global {label}_tilemap_height")
        lines.append(f"{label}_tilemap_height = {tiles_y}")
        lines.append("")

    # Palette
    if include_palette:
        lines.append(f".global {label}_palette")
        lines.append(f"{label}_palette:")

        for i in range(0, len(palette), 8):
            chunk = palette[i:i+8]
            hex_str = ", ".join(f"${c:04X}" for c in chunk)
            lines.append(f"    .word {hex_str}")

        lines.append(f"{label}_palette_end:")
        lines.append(f".global {label}_palette_size")
        lines.append(f"{label}_palette_size = {label}_palette_end - {label}_palette")
        lines.append(f".global {label}_palette_count")
        lines.append(f"{label}_palette_count = {len(palette)}")
        lines.append("")

    return "\n".join(lines)


def main():
    if not HAS_PIL:
        print("Error: PIL/Pillow is required. Install with: pip install Pillow", file=sys.stderr)
        sys.exit(1)

    parser = argparse.ArgumentParser(description="Convert PNG images to SNES tile data")
    parser.add_argument("input", help="Input PNG file")
    parser.add_argument("-o", "--output", help="Output file (default: stdout)")
    parser.add_argument("--mode", type=int, choices=[2, 4, 8], default=4,
                        help="Bits per pixel (default: 4)")
    parser.add_argument("--tilemap", action="store_true",
                        help="Generate tilemap data")
    parser.add_argument("--optimize", action="store_true",
                        help="Remove duplicate tiles")
    parser.add_argument("--palette", action="store_true",
                        help="Output palette data")
    parser.add_argument("--label", default="gfx",
                        help="Label prefix (default: gfx)")
    parser.add_argument("--raw", action="store_true",
                        help="Output raw binary instead of assembly")

    args = parser.parse_args()

    # Load image
    try:
        img = Image.open(args.input)
    except Exception as e:
        print(f"Error loading image: {e}", file=sys.stderr)
        sys.exit(1)

    # Check dimensions
    width, height = img.size
    if width % 8 != 0 or height % 8 != 0:
        print(f"Warning: Image size ({width}x{height}) is not a multiple of 8x8", file=sys.stderr)
        # Pad image
        new_w = ((width + 7) // 8) * 8
        new_h = ((height + 7) // 8) * 8
        new_img = Image.new(img.mode, (new_w, new_h), 0)
        new_img.paste(img, (0, 0))
        img = new_img
        width, height = img.size

    tiles_x = width // 8
    tiles_y = height // 8

    # Convert
    tiles, tilemap, palette = convert_image(img, args.mode, args.optimize or args.tilemap)

    print(f"Converted {args.input}: {len(tiles)} tiles, {len(palette)} colors", file=sys.stderr)

    if args.raw:
        # Output raw binary
        output = b''.join(tiles)
        if args.output:
            with open(args.output, 'wb') as f:
                f.write(output)
        else:
            sys.stdout.buffer.write(output)
    else:
        # Output assembly
        output = format_assembly(
            tiles, tilemap, palette,
            args.label,
            args.palette,
            args.tilemap or args.optimize,
            tiles_x, tiles_y
        )

        if args.output:
            with open(args.output, 'w') as f:
                f.write(output)
        else:
            print(output)


if __name__ == "__main__":
    main()
