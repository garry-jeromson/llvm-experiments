"""SNES ROM Builder.

This module provides the main SNESBuilder class for compiling, assembling,
and linking SNES ROMs from C/C++ or LLVM IR sources.
"""

import shutil
import subprocess
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import List, Optional

from .assembly_converter import convert_file as convert_assembly_file
from .rom_utils import (
    CartType,
    MIN_SUPERFX_ROM_SIZE,
    fix_checksum,
)


@dataclass
class BuildResult:
    """Result of a build operation."""
    success: bool
    output_path: Optional[Path] = None
    error: Optional[str] = None
    objects: List[Path] = field(default_factory=list)


class BuildError(Exception):
    """Error during build process."""
    pass


def find_project_root(start_path: Optional[Path] = None) -> Path:
    """Find the project root directory.

    Looks for directories containing CLAUDE.md, Makefile, or .git.

    Args:
        start_path: Starting path for search (default: current file's directory)

    Returns:
        Path to project root

    Raises:
        BuildError: If project root cannot be found
    """
    if start_path is None:
        start_path = Path(__file__).parent

    current = start_path.resolve()

    while current != current.parent:
        if (current / 'CLAUDE.md').exists() or \
           (current / 'Makefile').exists() or \
           (current / '.git').exists():
            return current
        current = current.parent

    raise BuildError("Could not find project root directory")


class SNESBuilder:
    """Builder for SNES ROMs.

    Supports building from C/C++ or LLVM IR sources using the W65816 LLVM
    backend, ca65 assembler, and ld65 linker.
    """

    def __init__(self, project_root: Optional[Path] = None, verbose: bool = True):
        """Initialize the builder.

        Args:
            project_root: Path to project root (auto-detected if not specified)
            verbose: If True, print status messages during build
        """
        self.project_root = project_root or find_project_root()
        self.verbose = verbose
        self.build_dir = self.project_root / "build" / "snes"

    def _log(self, message: str) -> None:
        """Print a log message if verbose mode is enabled."""
        if self.verbose:
            print(message)

    def _run(self, cmd: List[str], description: Optional[str] = None) -> subprocess.CompletedProcess:
        """Run a command and check for errors.

        Args:
            cmd: Command and arguments to run
            description: Description of the command for logging

        Returns:
            Completed process result

        Raises:
            BuildError: If command fails
        """
        if description:
            self._log(f"  {description}...")
        self._log(f"    $ {' '.join(str(c) for c in cmd)}")

        result = subprocess.run(cmd, capture_output=True, text=True)

        if result.returncode != 0:
            error_msg = f"Command failed: {' '.join(str(c) for c in cmd)}\n"
            if result.stdout:
                error_msg += f"stdout: {result.stdout}\n"
            if result.stderr:
                error_msg += f"stderr: {result.stderr}"
            raise BuildError(error_msg)

        return result

    def build(
        self,
        source: Path,
        output: Path,
        cart_type: str = "lorom",
        optimize: str = "-Os",
    ) -> BuildResult:
        """Build SNES ROM from C/C++ or LLVM IR source.

        Args:
            source: Path to source file (.c, .cpp, or .ll)
            output: Path to output ROM file (.sfc)
            cart_type: Cartridge type ("lorom" or "superfx")
            optimize: Optimization level for clang

        Returns:
            BuildResult with success status and output path
        """
        self.build_dir.mkdir(parents=True, exist_ok=True)

        source_path = Path(source).resolve()
        output_path = Path(output).resolve()
        source_basename = source_path.stem
        is_llvm_ir = source_path.suffix == '.ll'

        self._log(f"Building SNES ROM from {source}")
        self._log(f"Output: {output}")
        self._log(f"Cart type: {cart_type}")
        self._log("")

        try:
            # Step 1: Compile to LLVM IR (or use IR directly)
            if is_llvm_ir:
                self._log("Step 1: Using LLVM IR directly")
                ir_path = self.build_dir / f"{source_basename}.ll"
                shutil.copy(source_path, ir_path)
                self._log(f"  Copied {source} to {ir_path}")
            else:
                self._log("Step 1: Compile C/C++ to LLVM IR")
                ir_path = self.compile_to_ir(source_path, optimize)

            # Step 2: Compile IR to assembly
            self._log("\nStep 2: Compile LLVM IR to W65816 assembly")
            asm_path = self.compile_to_asm(ir_path)

            # Step 3: Convert assembly for ca65
            self._log("\nStep 3: Convert assembly to ca65 format")
            ca65_asm_path = self.convert_assembly(asm_path)
            self._log(f"  Created {ca65_asm_path}")

            # Step 4: Assemble user code
            self._log("\nStep 4: Assemble user code")
            user_obj = self.assemble(ca65_asm_path, self.build_dir / f"{source_basename}.o")

            # Step 5: Assemble crt0
            self._log("\nStep 5: Assemble SNES startup (crt0)")
            crt0_path = self.find_crt0(source_path.parent, cart_type)
            crt0_obj = self.assemble(crt0_path, self.build_dir / "crt0.o")

            # Collect objects to link
            link_objects = [crt0_obj]

            # Step 6: Assemble supporting files
            self._log("\nStep 6: Assemble supporting files")
            link_objects.extend(self._assemble_supporting_files(source_path))

            # Add user code last
            link_objects.append(user_obj)

            # Step 7: Link
            self._log("\nStep 7: Link ROM")
            linker_cfg = self.find_linker_config(source_path.parent, cart_type)
            self.link(link_objects, linker_cfg, output_path)

            # Step 8: Pad ROM if needed (SuperFX)
            if cart_type == "superfx":
                self._pad_superfx_rom(output_path)

            # Step 9: Fix checksum
            self._log("\nStep 8: Fix ROM checksum")
            fix_checksum(str(output_path), verbose=self.verbose)

            self._log(f"\nSuccess! ROM created: {output_path}")
            self._log(f"ROM size: {output_path.stat().st_size} bytes")

            return BuildResult(
                success=True,
                output_path=output_path,
                objects=link_objects,
            )

        except BuildError as e:
            return BuildResult(success=False, error=str(e))
        except Exception as e:
            return BuildResult(success=False, error=f"Unexpected error: {e}")

    def compile_to_ir(self, source: Path, optimize: str = "-Os") -> Path:
        """Compile C/C++ source to LLVM IR.

        Args:
            source: Path to source file
            optimize: Optimization level

        Returns:
            Path to generated IR file
        """
        ir_path = self.build_dir / f"{source.stem}.ll"
        clang = self.project_root / "build" / "bin" / "clang"

        self._run([
            str(clang),
            "-target", "w65816-unknown-none",
            optimize,
            "-S", "-emit-llvm",
            "-I", str(self.project_root / "snes"),
            "-I", str(self.project_root / "snes-sdk" / "include"),
            str(source),
            "-o", str(ir_path)
        ], "Running clang")

        return ir_path

    def compile_to_asm(self, ir_path: Path) -> Path:
        """Compile LLVM IR to W65816 assembly.

        Args:
            ir_path: Path to LLVM IR file

        Returns:
            Path to generated assembly file
        """
        asm_path = self.build_dir / f"{ir_path.stem}.s"
        llc = self.project_root / "build" / "bin" / "llc"

        self._run([
            str(llc),
            "-march=w65816",
            str(ir_path),
            "-o", str(asm_path)
        ], "Running llc")

        return asm_path

    def convert_assembly(self, asm_path: Path) -> Path:
        """Convert LLVM assembly to ca65 format.

        Args:
            asm_path: Path to LLVM assembly file

        Returns:
            Path to converted ca65 assembly file
        """
        return convert_assembly_file(asm_path)

    def assemble(self, asm_path: Path, output: Path) -> Path:
        """Assemble a file with ca65.

        Args:
            asm_path: Path to assembly file
            output: Path to output object file

        Returns:
            Path to object file
        """
        self._run([
            "ca65", "--cpu", "65816",
            "-o", str(output),
            str(asm_path)
        ], f"Assembling {asm_path.name}")

        return output

    def link(self, objects: List[Path], config: Path, output: Path) -> Path:
        """Link object files with ld65.

        Args:
            objects: List of object file paths
            config: Path to linker configuration file
            output: Path to output ROM file

        Returns:
            Path to output ROM file
        """
        self._run([
            "ld65",
            "-C", str(config),
            "-o", str(output)
        ] + [str(obj) for obj in objects], "Running ld65")

        return output

    def find_linker_config(self, source_dir: Path, cart_type: str) -> Path:
        """Find appropriate linker configuration file.

        Args:
            source_dir: Directory of source file
            cart_type: Cartridge type ("lorom" or "superfx")

        Returns:
            Path to linker config file

        Raises:
            BuildError: If no suitable config found
        """
        # Check for local config first
        if cart_type == "superfx":
            local_cfg = source_dir / "superfx.cfg"
            if local_cfg.exists():
                self._log(f"  Using local linker config: {local_cfg}")
                return local_cfg

        # Fall back to default
        default_cfg = self.project_root / "snes" / "lorom.cfg"
        if default_cfg.exists():
            self._log(f"  Using default linker config: {default_cfg}")
            return default_cfg

        raise BuildError(f"No linker config found for cart type: {cart_type}")

    def find_crt0(self, source_dir: Path, cart_type: str) -> Path:
        """Find appropriate crt0 startup file.

        Args:
            source_dir: Directory of source file
            cart_type: Cartridge type

        Returns:
            Path to crt0.s file

        Raises:
            BuildError: If no suitable crt0 found
        """
        # Check for local crt0 first
        local_crt0 = source_dir / "crt0.s"
        if local_crt0.exists():
            self._log(f"  Using local crt0.s from {source_dir}")
            return local_crt0

        # Fall back to default
        default_crt0 = self.project_root / "snes" / "crt0.s"
        if default_crt0.exists():
            self._log(f"  Using default crt0.s from snes/")
            return default_crt0

        raise BuildError("No crt0.s found")

    def _assemble_supporting_files(self, source_path: Path) -> List[Path]:
        """Assemble supporting files (fonts, sprites, runtime, data).

        Args:
            source_path: Path to main source file

        Returns:
            List of assembled object file paths
        """
        objects: List[Path] = []

        # Font file
        self._log("\n  Assembling base data files")
        font_paths = [
            self.project_root / "snes" / "font.s",
            self.project_root / "snes-sdk" / "data" / "font_2bpp.s",
        ]
        for font_path in font_paths:
            if font_path.exists():
                font_obj = self.build_dir / "font.o"
                self._run([
                    "ca65", "--cpu", "65816",
                    "-o", str(font_obj),
                    str(font_path)
                ], f"Assembling {font_path.name}")
                objects.append(font_obj)
                break

        # Sprites file
        sprites_path = self.project_root / "snes" / "sprites.s"
        if sprites_path.exists():
            sprites_obj = self.build_dir / "sprites.o"
            self._run([
                "ca65", "--cpu", "65816",
                "-o", str(sprites_obj),
                str(sprites_path)
            ], "Assembling sprites.s")
            objects.append(sprites_obj)

        # Project data files
        data_dir = source_path.parent / "data"
        if data_dir.exists() and data_dir.is_dir():
            self._log("\n  Assembling project data files")
            for data_file in sorted(data_dir.glob("*.s")):
                obj_path = self.build_dir / f"{data_file.stem}.o"
                self._run([
                    "ca65", "--cpu", "65816",
                    "-o", str(obj_path),
                    str(data_file)
                ], f"Assembling {data_file.name}")
                objects.append(obj_path)

        # Runtime library
        self._log("\n  Assembling runtime library")
        runtime_path = (self.project_root / "src" / "llvm-project" / "llvm" /
                       "lib" / "Target" / "W65816" / "runtime" / "w65816_runtime.s")
        if runtime_path.exists():
            runtime_obj = self.build_dir / "w65816_runtime.o"
            self._run([
                "ca65", "--cpu", "65816",
                "-o", str(runtime_obj),
                str(runtime_path)
            ], "Assembling runtime library")
            objects.append(runtime_obj)

        return objects

    def _pad_superfx_rom(self, output_path: Path) -> None:
        """Pad ROM to SuperFX minimum size (256KB).

        Args:
            output_path: Path to ROM file
        """
        current_size = output_path.stat().st_size
        if current_size < MIN_SUPERFX_ROM_SIZE:
            self._log(f"\n  Padding ROM from {current_size} to {MIN_SUPERFX_ROM_SIZE} bytes (SuperFX minimum)")
            with open(output_path, 'ab') as f:
                f.write(b'\x00' * (MIN_SUPERFX_ROM_SIZE - current_size))
