#!/usr/bin/env python3
"""
Build SNES ROM from C/C++ or LLVM IR source.

This is a CLI wrapper around the SNESBuilder class. For programmatic use,
import SNESBuilder directly from tools.snes_builder.

Pipeline:
1. Compile C/C++ to LLVM IR (using clang with w65816 target)
2. Compile LLVM IR to W65816 assembly
3. Convert assembly to ca65 format
4. Assemble with ca65
5. Link with crt0 using ld65
6. Fix ROM checksum

Usage:
    python build_rom.py <source> [output]
    python build_rom.py main.cpp output.sfc
    python build_rom.py test.ll game.sfc
"""

import argparse
import sys
from pathlib import Path

# Add project root to path for imports
script_dir = Path(__file__).parent.resolve()
project_root = script_dir.parent.parent
sys.path.insert(0, str(project_root))

from tools.snes_builder import SNESBuilder


def main():
    parser = argparse.ArgumentParser(
        description='Build SNES ROM from C/C++ or LLVM IR source'
    )
    parser.add_argument(
        'source',
        nargs='?',
        default='snes/demo.c',
        help='Source file (.c, .cpp, or .ll)'
    )
    parser.add_argument(
        'output',
        nargs='?',
        help='Output ROM file (.sfc). Default: build/snes/<source_stem>.sfc'
    )
    parser.add_argument(
        '--cart-type',
        choices=['lorom', 'superfx'],
        default='lorom',
        help='Cartridge type (default: lorom)'
    )
    parser.add_argument(
        '-O', '--optimize',
        default='-Os',
        help='Optimization level for clang (default: -Os)'
    )
    parser.add_argument(
        '-q', '--quiet',
        action='store_true',
        help='Suppress output messages'
    )

    args = parser.parse_args()

    source = Path(args.source)

    if args.output:
        output = Path(args.output)
    else:
        output = project_root / "build" / "snes" / f"{source.stem}.sfc"

    builder = SNESBuilder(project_root=project_root, verbose=not args.quiet)
    result = builder.build(
        source=source,
        output=output,
        cart_type=args.cart_type,
        optimize=args.optimize,
    )

    if not result.success:
        print(f"Build failed: {result.error}", file=sys.stderr)
        sys.exit(1)

    if not args.quiet:
        print(f"\nTo run: open {result.output_path}")


if __name__ == "__main__":
    main()
