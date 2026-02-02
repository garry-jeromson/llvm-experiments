"""GSU (SuperFX) assembler.

This module provides an assembler for GSU assembly language that parses
source code, resolves labels, and outputs raw bytes.
"""

import re
from typing import Dict, List, Optional, Tuple

from .gsu_instructions import (
    # Control
    STOP, NOP, CACHE, LOOP,
    # Shifts
    LSR, ROL, ASR, ROR, DIV2,
    # Branches
    BRA, BGE, BLT, BNE, BEQ, BPL, BMI, BCC, BCS, BVC, BVS,
    # Register prefixes
    TO, FROM, WITH, ALT1, ALT2, ALT3,
    # Memory
    STW, LDW, STB, LDB, SBK, LMS, SMS, LM, SM,
    # Plotting
    PLOT, RPIX, COLOR, CMODE,
    # Byte ops
    SWAP, NOT, HIB, LOB, SEX, MERGE,
    # Arithmetic
    ADD, SUB, ADC, SBC, CMP, MULT, UMULT, FMULT, LMULT, INC, DEC,
    # Logical
    AND, OR, XOR, BIC,
    # Immediate loads
    IBT, IWT,
    # ROM access
    GETB, GETBH, GETBL, GETBS, GETC, ROMB, RAMB,
    # Jumps
    JMP, LJMP, LINK,
)


class AssemblyError(Exception):
    """Error during assembly."""
    pass


class GSUAssembler:
    """Assembler for GSU (SuperFX) assembly language."""

    # Instructions with no operands
    NO_OPERAND_INSTRUCTIONS = {
        'STOP': STOP, 'NOP': NOP, 'CACHE': CACHE, 'LOOP': LOOP,
        'LSR': LSR, 'ROL': ROL, 'ASR': ASR, 'ROR': ROR, 'DIV2': DIV2,
        'PLOT': PLOT, 'RPIX': RPIX, 'COLOR': COLOR, 'CMODE': CMODE,
        'SWAP': SWAP, 'NOT': NOT, 'HIB': HIB, 'LOB': LOB, 'SEX': SEX, 'MERGE': MERGE,
        'SBK': SBK, 'FMULT': FMULT, 'LMULT': LMULT,
        'GETB': GETB, 'GETBH': GETBH, 'GETBL': GETBL, 'GETBS': GETBS,
        'GETC': GETC, 'ROMB': ROMB, 'RAMB': RAMB,
        'ALT1': ALT1, 'ALT2': ALT2, 'ALT3': ALT3,
    }

    # Instructions with single register operand (Rn)
    SINGLE_REG_INSTRUCTIONS = {
        'TO': TO, 'FROM': FROM, 'WITH': WITH,
        'INC': INC, 'DEC': DEC,
        'JMP': JMP, 'LJMP': LJMP,
    }

    # Instructions with register operand or immediate
    REG_OR_IMM_INSTRUCTIONS = {
        'ADD': ADD, 'SUB': SUB, 'MULT': MULT, 'UMULT': UMULT,
        'AND': AND, 'OR': OR, 'XOR': XOR, 'BIC': BIC,
    }

    # Instructions with register operand only (different class)
    REG_ONLY_INSTRUCTIONS = {
        'ADC': ADC, 'SBC': SBC, 'CMP': CMP,
    }

    # Memory instructions with (Rn)
    MEM_REG_INSTRUCTIONS = {
        'STW': STW, 'LDW': LDW, 'STB': STB, 'LDB': LDB,
    }

    # Branch instructions
    BRANCH_INSTRUCTIONS = {
        'BRA': BRA, 'BGE': BGE, 'BLT': BLT, 'BNE': BNE, 'BEQ': BEQ,
        'BPL': BPL, 'BMI': BMI, 'BCC': BCC, 'BCS': BCS, 'BVC': BVC, 'BVS': BVS,
    }

    def __init__(self):
        self.symbols: Dict[str, int] = {}
        self.origin: int = 0

    def assemble(self, source: str) -> bytes:
        """Assemble GSU source code to bytes.

        Args:
            source: GSU assembly source code

        Returns:
            Assembled machine code bytes

        Raises:
            AssemblyError: If there's an error during assembly
        """
        lines = source.split('\n')
        self.symbols = {}
        self.origin = 0

        # First pass: collect labels and determine instruction sizes
        instructions: List[Tuple[int, str, str]] = []  # (line_num, label, instruction)
        current_address = 0

        for line_num, line in enumerate(lines, 1):
            line = self._strip_comment(line).strip()
            if not line:
                continue

            # Check for .org directive
            if line.upper().startswith('.ORG'):
                self.origin = self._parse_number(line.split()[1])
                current_address = 0
                continue

            # Check for label
            label = None
            if ':' in line:
                parts = line.split(':', 1)
                label = parts[0].strip()
                self.symbols[label] = self.origin + current_address
                line = parts[1].strip() if len(parts) > 1 else ''

            if line:
                # Estimate instruction size for address calculation
                size = self._estimate_instruction_size(line)
                instructions.append((line_num, label, line))
                current_address += size

        # Second pass: assemble instructions with resolved labels
        output = bytearray()
        current_address = 0

        for line_num, label, line in instructions:
            if not line:
                continue

            try:
                code = self._assemble_line(line, self.origin + current_address)
                output.extend(code)
                current_address += len(code)
            except Exception as e:
                raise AssemblyError(f"Line {line_num}: {e}")

        return bytes(output)

    def assemble_instruction(self, line: str) -> bytes:
        """Assemble a single instruction.

        Args:
            line: Single instruction line

        Returns:
            Assembled bytes for the instruction
        """
        line = self._strip_comment(line).strip()
        return self._assemble_line(line, 0)

    def get_symbol(self, name: str) -> Optional[int]:
        """Get the address of a symbol.

        Args:
            name: Symbol name

        Returns:
            Address of the symbol, or None if not found
        """
        return self.symbols.get(name)

    def _strip_comment(self, line: str) -> str:
        """Remove comments from a line."""
        idx = line.find(';')
        if idx >= 0:
            return line[:idx]
        return line

    def _parse_number(self, s: str) -> int:
        """Parse a number from various formats."""
        s = s.strip()

        # Handle hash prefix for immediates
        if s.startswith('#'):
            s = s[1:]

        # Handle negative numbers
        negative = False
        if s.startswith('-'):
            negative = True
            s = s[1:]

        # $xx or $xxxx - hex with dollar prefix
        if s.startswith('$'):
            value = int(s[1:], 16)
        # 0x prefix
        elif s.startswith('0x') or s.startswith('0X'):
            value = int(s[2:], 16)
        # % prefix - binary
        elif s.startswith('%'):
            value = int(s[1:], 2)
        # 0b prefix - binary
        elif s.startswith('0b') or s.startswith('0B'):
            value = int(s[2:], 2)
        else:
            value = int(s)

        return -value if negative else value

    def _parse_register(self, s: str) -> int:
        """Parse a register number from 'Rn' or 'rn' format."""
        s = s.strip().upper()
        if s.startswith('R'):
            try:
                reg = int(s[1:])
                if 0 <= reg <= 15:
                    return reg
            except ValueError:
                pass
        raise AssemblyError(f"Invalid register: {s}")

    def _estimate_instruction_size(self, line: str) -> int:
        """Estimate the size of an instruction for the first pass."""
        parts = line.upper().split()
        if not parts:
            return 0

        mnemonic = parts[0]

        # Single byte instructions
        if mnemonic in self.NO_OPERAND_INSTRUCTIONS:
            # Some have ALT prefix
            if mnemonic in ('DIV2', 'RPIX', 'CMODE', 'LMULT', 'GETBH', 'GETBL', 'GETBS', 'ROMB', 'RAMB'):
                return 2
            return 1

        # Register prefix instructions
        if mnemonic in self.SINGLE_REG_INSTRUCTIONS:
            if mnemonic in ('LJMP',):
                return 2  # Has ALT1 prefix
            return 1

        # Memory instructions with (Rn)
        if mnemonic in self.MEM_REG_INSTRUCTIONS:
            if mnemonic in ('STB', 'LDB'):
                return 2  # Has ALT1 prefix
            return 1

        # ALT-prefixed register ops
        if mnemonic in self.REG_ONLY_INSTRUCTIONS:
            return 2

        # Register or immediate instructions
        if mnemonic in self.REG_OR_IMM_INSTRUCTIONS:
            if len(parts) > 1 and parts[1].startswith('#'):
                return 2  # ALT prefix for immediate
            if mnemonic in ('XOR', 'BIC'):
                return 2  # Always has ALT prefix
            return 1

        # Branch instructions - always 2 bytes
        if mnemonic in self.BRANCH_INSTRUCTIONS:
            return 2

        # IBT - 2 bytes
        if mnemonic == 'IBT':
            return 2

        # IWT - 3 bytes
        if mnemonic == 'IWT':
            return 3

        # LINK - 1 byte
        if mnemonic == 'LINK':
            return 1

        # LMS/SMS - 3 bytes
        if mnemonic in ('LMS', 'SMS'):
            return 3

        # LM/SM - 4 bytes
        if mnemonic in ('LM', 'SM'):
            return 4

        return 1  # Default

    def _assemble_line(self, line: str, current_address: int) -> bytes:
        """Assemble a single line of code."""
        parts = line.split(None, 1)
        if not parts:
            return bytes()

        mnemonic = parts[0].upper()
        operand = parts[1].strip() if len(parts) > 1 else ''

        # No-operand instructions
        if mnemonic in self.NO_OPERAND_INSTRUCTIONS:
            return self.NO_OPERAND_INSTRUCTIONS[mnemonic]().encode()

        # Single register instructions
        if mnemonic in self.SINGLE_REG_INSTRUCTIONS:
            if not operand:
                raise AssemblyError(f"{mnemonic} requires a register operand")
            reg = self._parse_register(operand)
            return self.SINGLE_REG_INSTRUCTIONS[mnemonic](reg).encode()

        # Memory instructions with (Rn)
        if mnemonic in self.MEM_REG_INSTRUCTIONS:
            if not operand:
                raise AssemblyError(f"{mnemonic} requires a register operand")
            # Parse (Rn) format
            match = re.match(r'\(?\s*R(\d+)\s*\)?', operand, re.IGNORECASE)
            if not match:
                raise AssemblyError(f"Invalid operand for {mnemonic}: {operand}")
            reg = int(match.group(1))
            return self.MEM_REG_INSTRUCTIONS[mnemonic](reg).encode()

        # Register-only ALT instructions
        if mnemonic in self.REG_ONLY_INSTRUCTIONS:
            if not operand:
                raise AssemblyError(f"{mnemonic} requires a register operand")
            reg = self._parse_register(operand)
            return self.REG_ONLY_INSTRUCTIONS[mnemonic](reg).encode()

        # Register or immediate instructions
        if mnemonic in self.REG_OR_IMM_INSTRUCTIONS:
            if not operand:
                raise AssemblyError(f"{mnemonic} requires an operand")
            if operand.startswith('#'):
                # Immediate mode
                value = self._parse_number(operand)
                return self.REG_OR_IMM_INSTRUCTIONS[mnemonic](value, immediate=True).encode()
            else:
                # Register mode
                reg = self._parse_register(operand)
                return self.REG_OR_IMM_INSTRUCTIONS[mnemonic](reg).encode()

        # Branch instructions
        if mnemonic in self.BRANCH_INSTRUCTIONS:
            if not operand:
                raise AssemblyError(f"{mnemonic} requires a target")

            # Check if operand is a label or a number
            try:
                # Try parsing as number first
                offset = self._parse_number(operand)
            except ValueError:
                # Must be a label
                if operand not in self.symbols:
                    raise AssemblyError(f"Undefined label: {operand}")
                target = self.symbols[operand]
                # Calculate relative offset: target - (current + 2)
                # The +2 accounts for the branch instruction itself
                offset = target - (current_address + 2)

            if offset < -128 or offset > 127:
                raise AssemblyError(f"Branch offset {offset} out of range (-128 to 127)")

            return self.BRANCH_INSTRUCTIONS[mnemonic](offset).encode()

        # IBT Rn, #xx
        if mnemonic == 'IBT':
            match = re.match(r'R(\d+)\s*,\s*(.+)', operand, re.IGNORECASE)
            if not match:
                raise AssemblyError(f"Invalid IBT syntax: {operand}")
            reg = int(match.group(1))
            value = self._parse_number(match.group(2))
            return IBT(reg, value).encode()

        # IWT Rn, #xxxx
        if mnemonic == 'IWT':
            match = re.match(r'R(\d+)\s*,\s*(.+)', operand, re.IGNORECASE)
            if not match:
                raise AssemblyError(f"Invalid IWT syntax: {operand}")
            reg = int(match.group(1))
            value = self._parse_number(match.group(2))
            return IWT(reg, value).encode()

        # LINK #n
        if mnemonic == 'LINK':
            value = self._parse_number(operand)
            return LINK(value).encode()

        # LMS Rn, (xx)
        if mnemonic == 'LMS':
            match = re.match(r'R(\d+)\s*,\s*\(\s*(.+?)\s*\)', operand, re.IGNORECASE)
            if not match:
                raise AssemblyError(f"Invalid LMS syntax: {operand}")
            reg = int(match.group(1))
            addr = self._parse_number(match.group(2))
            return LMS(reg, addr).encode()

        # SMS (xx), Rn
        if mnemonic == 'SMS':
            match = re.match(r'\(\s*(.+?)\s*\)\s*,\s*R(\d+)', operand, re.IGNORECASE)
            if not match:
                raise AssemblyError(f"Invalid SMS syntax: {operand}")
            addr = self._parse_number(match.group(1))
            reg = int(match.group(2))
            return SMS(reg, addr).encode()

        # LM Rn, (xxxx)
        if mnemonic == 'LM':
            match = re.match(r'R(\d+)\s*,\s*\(\s*(.+?)\s*\)', operand, re.IGNORECASE)
            if not match:
                raise AssemblyError(f"Invalid LM syntax: {operand}")
            reg = int(match.group(1))
            addr = self._parse_number(match.group(2))
            return LM(reg, addr).encode()

        # SM (xxxx), Rn
        if mnemonic == 'SM':
            match = re.match(r'\(\s*(.+?)\s*\)\s*,\s*R(\d+)', operand, re.IGNORECASE)
            if not match:
                raise AssemblyError(f"Invalid SM syntax: {operand}")
            addr = self._parse_number(match.group(1))
            reg = int(match.group(2))
            return SM(reg, addr).encode()

        raise AssemblyError(f"Unknown instruction: {mnemonic}")
