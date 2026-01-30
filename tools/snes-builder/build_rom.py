#!/usr/bin/env python3
"""
Build SNES ROM from C source using LLVM W65816 backend.

Pipeline:
1. Compile C to LLVM IR (using clang with msp430 target)
2. Fix triple to w65816
3. Compile LLVM IR to W65816 assembly
4. Assemble with ca65
5. Link with crt0 using ld65
6. Fix ROM checksum
"""

import subprocess
import sys
import os
import re
from pathlib import Path


def run(cmd: list, description: str = None):
    """Run a command and check for errors."""
    if description:
        print(f"  {description}...")
    print(f"    $ {' '.join(cmd)}")
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        print(f"Error running: {' '.join(cmd)}")
        print(f"stdout: {result.stdout}")
        print(f"stderr: {result.stderr}")
        sys.exit(1)
    return result


def fix_llvm_asm_for_ca65(asm_path: str) -> str:
    """
    Convert LLVM assembly output to ca65-compatible syntax.
    Returns path to the fixed assembly file.
    """
    with open(asm_path, 'r') as f:
        asm = f.read()

    # Remove LLVM directives that ca65 doesn't understand
    lines = []
    local_label_counter = 0

    for line in asm.split('\n'):
        stripped = line.strip()

        # Skip LLVM-specific directives
        if stripped.startswith('.file'):
            continue
        if stripped.startswith('.type'):
            continue
        if stripped.startswith('.size'):
            continue
        if stripped.startswith('.ident'):
            continue
        if stripped.startswith('.section') and '.note' in stripped:
            continue
        if stripped.startswith('.addrsig'):
            continue
        if stripped.startswith('.cfi_'):
            continue
        if stripped.startswith('.p2align'):
            # Convert .p2align N to .align N (ca65 uses byte count, not power of 2)
            # For simplicity, just skip alignment directives
            continue

        # Convert LLVM local labels (starting with .L) to ca65 format
        # ca65 doesn't like dots in label names, so remove the leading dot
        if stripped.startswith('.L') and stripped.endswith(':'):
            # .LBB0_1: -> LBB0_1:
            label = stripped[1:]  # Remove leading dot
            lines.append(label)
            continue

        # Skip .Lfunc_end labels (function size markers not needed)
        if '.Lfunc_end' in stripped:
            continue

        # Convert references to .LBB labels in branch instructions
        # e.g., "bra .LBB0_1" -> "bra LBB0_1"
        if '.LBB' in stripped:
            line = line.replace('.LBB', 'LBB')
            lines.append(line)
            continue

        # Convert .section to ca65 .segment
        if stripped.startswith('.section'):
            # .section .text -> .segment "CODE"
            if '.text' in stripped:
                lines.append('.segment "CODE"')
                continue
            elif '.rodata' in stripped:
                lines.append('.segment "RODATA"')
                continue
            elif '.data' in stripped:
                lines.append('.segment "DATA"')
                continue
            elif '.bss' in stripped:
                lines.append('.segment "BSS"')
                continue

        # Convert .text directive
        if stripped == '.text':
            lines.append('.segment "CODE"')
            continue

        # Convert .globl to .export
        if stripped.startswith('.globl '):
            symbol = stripped.split()[1]
            lines.append(f'.export {symbol}')
            continue

        # Convert .global to .export (ca65 uses .export, not .global)
        if stripped.startswith('.global '):
            symbol = stripped.split()[1]
            lines.append(f'.export {symbol}')
            continue

        # Handle labels (remove @ prefix if present, keep _ prefix)
        if stripped.endswith(':') and not stripped.startswith('.'):
            label = stripped[:-1]
            # LLVM sometimes uses @ for local labels
            if label.startswith('@'):
                label = label[1:]
            lines.append(f'{label}:')
            continue

        # Handle comments - convert LLVM-style to ca65-style
        # Lines that are only comments starting with ;
        if stripped.startswith(';'):
            lines.append(line)
            continue

        # Convert @symbol references in instructions
        if '@' in line and not stripped.startswith(';'):
            line = line.replace('@', '')

        lines.append(line)

    # Add necessary ca65 directives at the top
    # Include imports for runtime library functions
    header = """.p816
.smart
.a16
.i16

; Runtime library imports (for mul, div, mod operations)
.import __mulhi3
.import __divhi3
.import __udivhi3
.import __modhi3
.import __umodhi3

"""
    fixed_asm = header + '\n'.join(lines)

    # Write fixed assembly
    fixed_path = asm_path.replace('.s', '.ca65.s')
    with open(fixed_path, 'w') as f:
        f.write(fixed_asm)

    return fixed_path


def build(source_file: str, output_sfc: str):
    """Build SNES ROM from C source or LLVM IR."""

    # Get project root directory
    script_dir = Path(__file__).parent.resolve()
    project_root = script_dir.parent.parent

    build_dir = project_root / "build" / "snes"
    build_dir.mkdir(parents=True, exist_ok=True)

    source_path = Path(source_file).resolve()
    source_basename = source_path.stem
    is_llvm_ir = source_path.suffix == '.ll'

    print(f"Building SNES ROM from {source_file}")
    print(f"Output: {output_sfc}")
    print()

    ir_path = build_dir / f"{source_basename}.ll"

    if is_llvm_ir:
        # Direct LLVM IR input - just copy it
        print("Step 1: Using LLVM IR directly")
        import shutil
        shutil.copy(source_path, ir_path)
        print(f"  Copied {source_file} to {ir_path}")
        print("\nStep 2: Skipped (already LLVM IR)")
    else:
        # Compile C to LLVM IR
        print("Step 1: Compile C to LLVM IR")
        run([
            str(project_root / "build" / "bin" / "clang"),
            "-target", "msp430-unknown-none",
            "-O2",
            "-S", "-emit-llvm",
            "-I", str(project_root / "snes"),
            str(source_path),
            "-o", str(ir_path)
        ], "Running clang")

        # Fix triple
        print("\nStep 2: Fix target triple")
        with open(ir_path, 'r') as f:
            ir = f.read()
        ir = ir.replace('msp430-unknown-none', 'w65816-unknown-none')
        ir = ir.replace('target datalayout = "e-m:e-p:16:16-i32:16-i64:16-f32:16-f64:16-a:8-n8:16-S16"',
                        'target datalayout = "e-m:e-p:16:16-i32:16-i64:16-f32:16-f64:16-a:8-n8:16-S16"')
        with open(ir_path, 'w') as f:
            f.write(ir)
        print("  Triple changed to w65816-unknown-none")

    c_basename = source_basename  # For compatibility with rest of function

    # Step 3: Compile LLVM IR to assembly
    print("\nStep 3: Compile LLVM IR to W65816 assembly")
    asm_path = build_dir / f"{c_basename}.s"
    run([
        str(project_root / "build" / "bin" / "llc"),
        "-march=w65816",
        str(ir_path),
        "-o", str(asm_path)
    ], "Running llc")

    # Step 4: Fix assembly for ca65
    print("\nStep 4: Convert assembly to ca65 format")
    ca65_asm_path = fix_llvm_asm_for_ca65(str(asm_path))
    print(f"  Created {ca65_asm_path}")

    # Step 5: Assemble user code with ca65
    print("\nStep 5: Assemble user code")
    user_obj = build_dir / f"{c_basename}.o"
    run([
        "ca65", "--cpu", "65816",
        "-o", str(user_obj),
        ca65_asm_path
    ], "Running ca65 on user code")

    # Step 6: Assemble crt0
    print("\nStep 6: Assemble SNES startup (crt0)")
    crt0_obj = build_dir / "crt0.o"
    run([
        "ca65", "--cpu", "65816",
        "-o", str(crt0_obj),
        str(project_root / "snes" / "crt0.s")
    ], "Running ca65 on crt0")

    # Step 6b: Assemble font data
    print("\nStep 6b: Assemble font data")
    font_obj = build_dir / "font.o"
    font_path = project_root / "snes" / "font.s"
    link_objects = [str(crt0_obj)]
    if font_path.exists():
        run([
            "ca65", "--cpu", "65816",
            "-o", str(font_obj),
            str(font_path)
        ], "Running ca65 on font")
        link_objects.append(str(font_obj))
    else:
        print("  No font.s found, skipping font assembly")

    # Step 6c: Assemble sprite data
    print("\nStep 6c: Assemble sprite data")
    sprites_obj = build_dir / "sprites.o"
    sprites_path = project_root / "snes" / "sprites.s"
    if sprites_path.exists():
        run([
            "ca65", "--cpu", "65816",
            "-o", str(sprites_obj),
            str(sprites_path)
        ], "Running ca65 on sprites")
        link_objects.append(str(sprites_obj))
    else:
        print("  No sprites.s found, skipping sprite assembly")

    # Step 6d: Assemble runtime library (for mul, div, etc.)
    print("\nStep 6d: Assemble runtime library")
    runtime_obj = build_dir / "w65816_runtime.o"
    runtime_path = project_root / "src" / "llvm-project" / "llvm" / "lib" / "Target" / "W65816" / "runtime" / "w65816_runtime.s"
    if runtime_path.exists():
        run([
            "ca65", "--cpu", "65816",
            "-o", str(runtime_obj),
            str(runtime_path)
        ], "Running ca65 on runtime library")
        link_objects.append(str(runtime_obj))
    else:
        print("  No runtime library found, skipping")

    # Add user code last
    link_objects.append(str(user_obj))

    # Step 7: Link
    print("\nStep 7: Link ROM")
    output_path = Path(output_sfc).resolve()
    run([
        "ld65",
        "-C", str(project_root / "snes" / "lorom.cfg"),
        "-o", str(output_path)
    ] + link_objects, "Running ld65")

    # Step 8: Fix checksum
    print("\nStep 8: Fix ROM checksum")
    run([
        "python3",
        str(script_dir / "fix_checksum.py"),
        str(output_path)
    ], "Fixing checksum")

    print(f"\nSuccess! ROM created: {output_path}")
    print(f"ROM size: {output_path.stat().st_size} bytes")
    print(f"\nTo run: open {output_path}")


def main():
    if len(sys.argv) < 2:
        # Default: build snes/demo.c
        c_file = "snes/demo.c"
        output = "build/snes/demo.sfc"
    elif len(sys.argv) == 2:
        c_file = sys.argv[1]
        output = f"build/snes/{Path(c_file).stem}.sfc"
    else:
        c_file = sys.argv[1]
        output = sys.argv[2]

    build(c_file, output)


if __name__ == "__main__":
    main()
