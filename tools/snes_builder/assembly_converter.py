"""LLVM to ca65 assembly converter.

This module converts LLVM-generated W65816 assembly to ca65-compatible
format, handling directive conversion, symbol tracking, and import generation.
"""

from dataclasses import dataclass, field
from pathlib import Path
from typing import Set, List, Tuple


# Runtime library symbols that may need to be imported
RUNTIME_SYMBOLS = [
    '__mulhi3',
    '__divhi3',
    '__udivhi3',
    '__modhi3',
    '__umodhi3',
]

# Instructions that can reference external symbols
SYMBOL_REFERENCING_OPCODES = frozenset([
    'jsr', 'jsl', 'jmp', 'jml',
    'lda', 'sta', 'ldx', 'stx', 'ldy', 'sty',
    'inc', 'dec', 'adc', 'sbc',
    'and', 'ora', 'eor',
    'cmp', 'cpx', 'cpy',
    'bit', 'tsb', 'trb',
    'asl', 'lsr', 'rol', 'ror',
])


@dataclass
class ConversionResult:
    """Result of assembly conversion."""
    output: str
    defined_symbols: Set[str] = field(default_factory=set)
    referenced_symbols: Set[str] = field(default_factory=set)
    external_symbols: Set[str] = field(default_factory=set)


def collect_symbols(asm_lines: List[str]) -> Tuple[Set[str], Set[str]]:
    """Collect defined and referenced symbols from assembly.

    Args:
        asm_lines: Lines of LLVM assembly

    Returns:
        Tuple of (defined_symbols, referenced_symbols)
    """
    defined_symbols: Set[str] = set()
    referenced_symbols: Set[str] = set()

    for line in asm_lines:
        stripped = line.strip()

        # Labels define symbols
        if stripped.endswith(':') and not stripped.startswith('.'):
            label = stripped[:-1]
            if label.startswith('@'):
                label = label[1:]
            defined_symbols.add(label)

        # .comm directives define symbols
        elif stripped.startswith('.comm'):
            parts = stripped.replace(',', ' ').split()
            if len(parts) >= 2:
                defined_symbols.add(parts[1])

        # .local directives mark symbols as local
        elif stripped.startswith('.local'):
            parts = stripped.split()
            if len(parts) >= 2:
                defined_symbols.add(parts[1])

        # Check for symbol references in instructions
        else:
            parts = stripped.split()
            if len(parts) >= 2:
                opcode = parts[0].lower()
                operand = parts[1].rstrip(',')

                # Extract symbol from operand
                symbol = _extract_symbol_from_operand(operand)

                if symbol and opcode in SYMBOL_REFERENCING_OPCODES:
                    referenced_symbols.add(symbol)

    return defined_symbols, referenced_symbols


def _extract_symbol_from_operand(operand: str) -> str:
    """Extract symbol name from an instruction operand.

    Args:
        operand: Instruction operand string

    Returns:
        Symbol name or empty string if no valid symbol found
    """
    # Remove addressing mode wrappers
    if operand.startswith('('):
        operand = operand[1:].split(')')[0].split(',')[0]

    # Remove immediate mode prefix
    if operand.startswith('#'):
        operand = operand[1:]

    # Remove offset part (e.g., "symbol+2" -> "symbol")
    if '+' in operand:
        operand = operand.split('+')[0]
    if '-' in operand:
        operand = operand.split('-')[0]

    # Check if it looks like a valid external symbol:
    # - Must start with _ (C mangled names)
    # - Must be longer than 1 character
    # - Must not be a numeric constant
    if (operand
            and operand.startswith('_')
            and len(operand) > 1
            and not operand.startswith('$')):
        return operand

    return ''


def convert_llvm_to_ca65(asm_content: str) -> ConversionResult:
    """Convert LLVM assembly output to ca65-compatible syntax.

    Args:
        asm_content: LLVM-generated assembly content

    Returns:
        ConversionResult with converted assembly and symbol information
    """
    lines = asm_content.split('\n')

    # Collect symbols
    defined_symbols, referenced_symbols = collect_symbols(lines)

    # Find external symbols
    external_symbols = referenced_symbols - defined_symbols

    # Convert lines
    output_lines: List[str] = []

    for line in lines:
        converted = _convert_line(line)
        if converted is not None:
            output_lines.append(converted)

    # Build header with imports
    header = _build_header(external_symbols)

    return ConversionResult(
        output=header + '\n'.join(output_lines),
        defined_symbols=defined_symbols,
        referenced_symbols=referenced_symbols,
        external_symbols=external_symbols,
    )


def _convert_line(line: str) -> str | None:
    """Convert a single line of LLVM assembly to ca65 format.

    Args:
        line: Single line of LLVM assembly

    Returns:
        Converted line, or None if line should be skipped
    """
    stripped = line.strip()

    # Skip LLVM-specific directives
    skip_prefixes = (
        '.file',
        '.type',
        '.size',
        '.ident',
        '.addrsig',
        '.cfi_',
        '.p2align',
        '.local',
    )
    if any(stripped.startswith(p) for p in skip_prefixes):
        return None

    # Skip .section .note
    if stripped.startswith('.section') and '.note' in stripped:
        return None

    # Skip .Lfunc_end labels
    if '.Lfunc_end' in stripped:
        return None

    # Convert .comm directive
    if stripped.startswith('.comm'):
        parts = stripped.replace(',', ' ').split()
        if len(parts) >= 3:
            symbol = parts[1]
            size = parts[2]
            return f'.segment "BSS"\n{symbol}: .res {size}'
        return None

    # Convert .zero directive
    if stripped.startswith('.zero'):
        parts = stripped.split()
        if len(parts) >= 2:
            size = parts[1]
            return f'\t.res {size}'
        return None

    # Convert .L local labels
    if stripped.startswith('.L') and stripped.endswith(':'):
        return stripped[1:]  # Remove leading dot

    # Convert .section to .segment
    if stripped.startswith('.section'):
        return _convert_section_directive(stripped)

    # Convert .text directive
    if stripped == '.text':
        return '.segment "CODE"'

    # Convert .globl/.global to .export
    if stripped.startswith('.globl ') or stripped.startswith('.global '):
        symbol = stripped.split()[1]
        return f'.export {symbol}'

    # Handle regular labels (remove @ prefix)
    if stripped.endswith(':') and not stripped.startswith('.'):
        label = stripped[:-1]
        if label.startswith('@'):
            label = label[1:]
        return f'{label}:'

    # Convert label references
    result = line
    if '.LBB' in result:
        result = result.replace('.LBB', 'LBB')
    if '.Ltmp' in result:
        result = result.replace('.Ltmp', 'Ltmp')
    if '@' in result and not stripped.startswith(';'):
        result = result.replace('@', '')

    return result


def _convert_section_directive(directive: str) -> str | None:
    """Convert .section directive to ca65 .segment.

    Args:
        directive: .section directive string

    Returns:
        Converted .segment directive or None
    """
    section_map = {
        '.text': '.segment "CODE"',
        '.rodata': '.segment "RODATA"',
        '.data': '.segment "DATA"',
        '.bss': '.segment "BSS"',
    }

    for section, segment in section_map.items():
        if section in directive:
            return segment

    return None


def _build_header(external_symbols: Set[str]) -> str:
    """Build ca65 header with imports.

    Args:
        external_symbols: Set of external symbols to import

    Returns:
        Header string
    """
    header_lines = [
        '.p816',
        '.smart',
        '.a16',
        '.i16',
        '',
        '; Runtime library imports (for mul, div, mod operations)',
    ]

    # Add runtime library imports
    for sym in RUNTIME_SYMBOLS:
        header_lines.append(f'.import {sym}')

    header_lines.append('')

    # Add external symbol imports
    if external_symbols:
        header_lines.append('; External symbol imports')
        for sym in sorted(external_symbols):
            header_lines.append(f'.import {sym}')
        header_lines.append('')

    return '\n'.join(header_lines) + '\n'


def convert_file(input_path: Path, output_path: Path | None = None) -> Path:
    """Convert LLVM assembly file to ca65 format.

    Args:
        input_path: Path to LLVM assembly file
        output_path: Output path (default: replaces .s with .ca65.s)

    Returns:
        Path to converted assembly file
    """
    if output_path is None:
        output_path = input_path.with_suffix('.ca65.s')

    with open(input_path, 'r') as f:
        asm_content = f.read()

    result = convert_llvm_to_ca65(asm_content)

    with open(output_path, 'w') as f:
        f.write(result.output)

    return output_path
