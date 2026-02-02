"""SNES ROM Builder package.

This package provides tools for building SNES ROMs from C/C++ or LLVM IR
sources using the W65816 LLVM backend.

Example usage:
    from rom_builder import SNESBuilder

    builder = SNESBuilder()
    result = builder.build(
        source=Path("main.cpp"),
        output=Path("game.sfc"),
        cart_type="lorom"
    )
    if result.success:
        print(f"ROM created: {result.output_path}")

For SuperFX ROMs:
    from rom_builder.superfx import GSUAssembler, SuperFXROMBuilder

    assembler = GSUAssembler()
    code = assembler.assemble("STOP")
    builder = SuperFXROMBuilder(code)
    rom = builder.build()
"""

from .builder import SNESBuilder, BuildResult, BuildError, find_project_root
from .assembly_converter import (
    convert_llvm_to_ca65,
    convert_file as convert_assembly_file,
    ConversionResult,
)
from .rom_utils import (
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
    HIROM_HEADER_OFFSET,
    MIN_ROM_SIZE,
    MIN_SUPERFX_ROM_SIZE,
)

# SuperFX submodule imports
from .superfx import (
    GSUAssembler,
    AssemblyError,
    SuperFXROMBuilder,
)

__all__ = [
    # Main builder
    'SNESBuilder',
    'BuildResult',
    'BuildError',
    'find_project_root',

    # Assembly converter
    'convert_llvm_to_ca65',
    'convert_assembly_file',
    'ConversionResult',

    # ROM utilities
    'CartType',
    'MapMode',
    'ROMSize',
    'RAMSize',
    'calculate_checksum',
    'calculate_checksum_complement',
    'create_header',
    'write_header',
    'write_checksum',
    'fix_checksum',
    'pad_rom',
    'get_rom_size_code',
    'LOROM_HEADER_OFFSET',
    'HIROM_HEADER_OFFSET',
    'MIN_ROM_SIZE',
    'MIN_SUPERFX_ROM_SIZE',

    # SuperFX
    'GSUAssembler',
    'AssemblyError',
    'SuperFXROMBuilder',
]
