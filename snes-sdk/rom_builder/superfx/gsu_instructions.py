"""GSU (SuperFX) instruction definitions and encoding.

This module provides classes for all GSU instructions that can encode
themselves to bytes for assembly.
"""

from typing import Union


def _validate_register(reg: int, min_reg: int = 0, max_reg: int = 15, name: str = "register"):
    """Validate register number is within range."""
    if not isinstance(reg, int) or reg < min_reg or reg > max_reg:
        raise ValueError(f"{name} must be between {min_reg} and {max_reg}, got {reg}")


def _validate_branch_offset(offset: int):
    """Validate branch offset is within signed 8-bit range."""
    if not isinstance(offset, int) or offset < -128 or offset > 127:
        raise ValueError(f"Branch offset must be between -128 and 127, got {offset}")


def _to_signed_byte(value: int) -> int:
    """Convert signed value to unsigned byte (two's complement)."""
    if value < 0:
        return (256 + value) & 0xFF
    return value & 0xFF


# =============================================================================
# Base Instruction Class
# =============================================================================

class Instruction:
    """Base class for GSU instructions."""

    def encode(self) -> bytes:
        """Encode the instruction to bytes."""
        raise NotImplementedError("Subclasses must implement encode()")


# =============================================================================
# Control Instructions
# =============================================================================

class STOP(Instruction):
    """STOP - Stop GSU execution, set IRQ flag."""

    def encode(self) -> bytes:
        return bytes([0x00])


class NOP(Instruction):
    """NOP - No operation."""

    def encode(self) -> bytes:
        return bytes([0x01])


class CACHE(Instruction):
    """CACHE - Set cache base, invalidate cache."""

    def encode(self) -> bytes:
        return bytes([0x02])


class LOOP(Instruction):
    """LOOP - Decrement R12, branch to R13 if != 0."""

    def encode(self) -> bytes:
        return bytes([0x3C])


# =============================================================================
# Shift Instructions
# =============================================================================

class LSR(Instruction):
    """LSR - Logical shift right."""

    def encode(self) -> bytes:
        return bytes([0x03])


class ROL(Instruction):
    """ROL - Rotate left through carry."""

    def encode(self) -> bytes:
        return bytes([0x04])


class ASR(Instruction):
    """ASR - Arithmetic shift right (preserves sign)."""

    def encode(self) -> bytes:
        return bytes([0x96])


class ROR(Instruction):
    """ROR - Rotate right through carry."""

    def encode(self) -> bytes:
        return bytes([0x97])


class DIV2(Instruction):
    """DIV2 - Divide by 2 (arithmetic). ALT1 variant of ASR."""

    def encode(self) -> bytes:
        return bytes([0x3D, 0x96])  # ALT1 + ASR


# =============================================================================
# Branch Instructions
# =============================================================================

class _BranchInstruction(Instruction):
    """Base class for branch instructions."""

    opcode: int = 0

    def __init__(self, offset: int):
        _validate_branch_offset(offset)
        self.offset = offset

    def encode(self) -> bytes:
        return bytes([self.opcode, _to_signed_byte(self.offset)])


class BRA(_BranchInstruction):
    """BRA - Branch always."""
    opcode = 0x05


class BGE(_BranchInstruction):
    """BGE - Branch if >= 0 (signed)."""
    opcode = 0x06


class BLT(_BranchInstruction):
    """BLT - Branch if < 0 (signed)."""
    opcode = 0x07


class BNE(_BranchInstruction):
    """BNE - Branch if not equal (Z=0)."""
    opcode = 0x08


class BEQ(_BranchInstruction):
    """BEQ - Branch if equal (Z=1)."""
    opcode = 0x09


class BPL(_BranchInstruction):
    """BPL - Branch if plus (S=0)."""
    opcode = 0x0A


class BMI(_BranchInstruction):
    """BMI - Branch if minus (S=1)."""
    opcode = 0x0B


class BCC(_BranchInstruction):
    """BCC - Branch if carry clear."""
    opcode = 0x0C


class BCS(_BranchInstruction):
    """BCS - Branch if carry set."""
    opcode = 0x0D


class BVC(_BranchInstruction):
    """BVC - Branch if overflow clear."""
    opcode = 0x0E


class BVS(_BranchInstruction):
    """BVS - Branch if overflow set."""
    opcode = 0x0F


# =============================================================================
# Register Selection Prefixes
# =============================================================================

class TO(Instruction):
    """TO Rn - Set destination register for next op."""

    def __init__(self, reg: int):
        _validate_register(reg)
        self.reg = reg

    def encode(self) -> bytes:
        return bytes([0x10 + self.reg])


class WITH(Instruction):
    """WITH Rn - Set both source and destination."""

    def __init__(self, reg: int):
        _validate_register(reg)
        self.reg = reg

    def encode(self) -> bytes:
        return bytes([0x20 + self.reg])


class FROM(Instruction):
    """FROM Rn - Set source register for next op."""

    def __init__(self, reg: int):
        _validate_register(reg)
        self.reg = reg

    def encode(self) -> bytes:
        return bytes([0xB0 + self.reg])


class ALT1(Instruction):
    """ALT1 - Enable ALT1 mode for next instruction."""

    def encode(self) -> bytes:
        return bytes([0x3D])


class ALT2(Instruction):
    """ALT2 - Enable ALT2 mode for next instruction."""

    def encode(self) -> bytes:
        return bytes([0x3E])


class ALT3(Instruction):
    """ALT3 - Enable ALT1+ALT2 for next instruction."""

    def encode(self) -> bytes:
        return bytes([0x3F])


# =============================================================================
# Memory Instructions
# =============================================================================

class STW(Instruction):
    """STW (Rm) - Store word to RAM[Rm]."""

    def __init__(self, reg: int):
        _validate_register(reg, 0, 11, "STW register")
        self.reg = reg

    def encode(self) -> bytes:
        return bytes([0x30 + self.reg])


class LDW(Instruction):
    """LDW (Rm) - Load word from RAM[Rm]."""

    def __init__(self, reg: int):
        _validate_register(reg, 0, 11, "LDW register")
        self.reg = reg

    def encode(self) -> bytes:
        return bytes([0x40 + self.reg])


class STB(Instruction):
    """STB (Rm) - Store byte to RAM[Rm]. ALT1 variant of STW."""

    def __init__(self, reg: int):
        _validate_register(reg, 0, 11, "STB register")
        self.reg = reg

    def encode(self) -> bytes:
        return bytes([0x3D, 0x30 + self.reg])  # ALT1 + STW


class LDB(Instruction):
    """LDB (Rm) - Load byte from RAM[Rm]. ALT1 variant of LDW."""

    def __init__(self, reg: int):
        _validate_register(reg, 0, 11, "LDB register")
        self.reg = reg

    def encode(self) -> bytes:
        return bytes([0x3D, 0x40 + self.reg])  # ALT1 + LDW


class SBK(Instruction):
    """SBK - Store word to last RAM address."""

    def encode(self) -> bytes:
        return bytes([0x90])


class LMS(Instruction):
    """LMS Rn, (xx) - Load word from RAM short address."""

    def __init__(self, reg: int, addr: int):
        _validate_register(reg)
        self.reg = reg
        self.addr = addr & 0xFF

    def encode(self) -> bytes:
        return bytes([0x3D, 0xA0 + self.reg, self.addr])  # ALT1 + IBT opcode + addr


class SMS(Instruction):
    """SMS (xx), Rn - Store word to RAM short address."""

    def __init__(self, reg: int, addr: int):
        _validate_register(reg)
        self.reg = reg
        self.addr = addr & 0xFF

    def encode(self) -> bytes:
        return bytes([0x3E, 0xA0 + self.reg, self.addr])  # ALT2 + IBT opcode + addr


class LM(Instruction):
    """LM Rn, (xxxx) - Load word from RAM address."""

    def __init__(self, reg: int, addr: int):
        _validate_register(reg)
        self.reg = reg
        self.addr = addr & 0xFFFF

    def encode(self) -> bytes:
        lo = self.addr & 0xFF
        hi = (self.addr >> 8) & 0xFF
        return bytes([0x3D, 0xF0 + self.reg, lo, hi])  # ALT1 + IWT opcode + addr


class SM(Instruction):
    """SM (xxxx), Rn - Store word to RAM address."""

    def __init__(self, reg: int, addr: int):
        _validate_register(reg)
        self.reg = reg
        self.addr = addr & 0xFFFF

    def encode(self) -> bytes:
        lo = self.addr & 0xFF
        hi = (self.addr >> 8) & 0xFF
        return bytes([0x3E, 0xF0 + self.reg, lo, hi])  # ALT2 + IWT opcode + addr


# =============================================================================
# Plotting Instructions
# =============================================================================

class PLOT(Instruction):
    """PLOT - Plot pixel at (R1, R2) with COLR."""

    def encode(self) -> bytes:
        return bytes([0x4C])


class RPIX(Instruction):
    """RPIX - Read pixel at (R1, R2) to Dreg. ALT1 variant of PLOT."""

    def encode(self) -> bytes:
        return bytes([0x3D, 0x4C])  # ALT1 + PLOT


class COLOR(Instruction):
    """COLOR - Set COLR from low byte of Sreg."""

    def encode(self) -> bytes:
        return bytes([0x4E])


class CMODE(Instruction):
    """CMODE - Set plot options from Sreg. ALT1 variant of COLOR."""

    def encode(self) -> bytes:
        return bytes([0x3D, 0x4E])  # ALT1 + COLOR


# =============================================================================
# Byte Instructions
# =============================================================================

class SWAP(Instruction):
    """SWAP - Swap high and low bytes."""

    def encode(self) -> bytes:
        return bytes([0x4D])


class NOT(Instruction):
    """NOT - Dreg = NOT Sreg."""

    def encode(self) -> bytes:
        return bytes([0x4F])


class HIB(Instruction):
    """HIB - Dreg = high byte of Sreg."""

    def encode(self) -> bytes:
        return bytes([0xC0])


class LOB(Instruction):
    """LOB - Dreg = low byte of Sreg."""

    def encode(self) -> bytes:
        return bytes([0x9E])


class SEX(Instruction):
    """SEX - Sign extend byte to word."""

    def encode(self) -> bytes:
        return bytes([0x95])


class MERGE(Instruction):
    """MERGE - Dreg = R7[15:8] : R8[15:8]."""

    def encode(self) -> bytes:
        return bytes([0x70])


# =============================================================================
# Arithmetic Instructions
# =============================================================================

class ADD(Instruction):
    """ADD Rn - Dreg = Sreg + Rn. With immediate=True: ADD #n."""

    def __init__(self, reg_or_imm: int, immediate: bool = False):
        _validate_register(reg_or_imm)
        self.reg = reg_or_imm
        self.immediate = immediate

    def encode(self) -> bytes:
        if self.immediate:
            return bytes([0x3D, 0x50 + self.reg])  # ALT1 + ADD
        return bytes([0x50 + self.reg])


class SUB(Instruction):
    """SUB Rn - Dreg = Sreg - Rn. With immediate=True: SUB #n."""

    def __init__(self, reg_or_imm: int, immediate: bool = False):
        _validate_register(reg_or_imm)
        self.reg = reg_or_imm
        self.immediate = immediate

    def encode(self) -> bytes:
        if self.immediate:
            return bytes([0x3D, 0x60 + self.reg])  # ALT1 + SUB
        return bytes([0x60 + self.reg])


class ADC(Instruction):
    """ADC Rn - Dreg = Sreg + Rn + Carry. ALT2 variant of ADD."""

    def __init__(self, reg: int):
        _validate_register(reg)
        self.reg = reg

    def encode(self) -> bytes:
        return bytes([0x3E, 0x50 + self.reg])  # ALT2 + ADD


class SBC(Instruction):
    """SBC Rn - Dreg = Sreg - Rn - ~Carry. ALT2 variant of SUB."""

    def __init__(self, reg: int):
        _validate_register(reg)
        self.reg = reg

    def encode(self) -> bytes:
        return bytes([0x3E, 0x60 + self.reg])  # ALT2 + SUB


class CMP(Instruction):
    """CMP Rn - Compare Sreg - Rn (flags only). ALT3 variant of SUB."""

    def __init__(self, reg: int):
        _validate_register(reg)
        self.reg = reg

    def encode(self) -> bytes:
        return bytes([0x3F, 0x60 + self.reg])  # ALT3 + SUB


class MULT(Instruction):
    """MULT Rn - Dreg = Sreg * Rn (signed 8x8). With immediate=True: MULT #n."""

    def __init__(self, reg_or_imm: int, immediate: bool = False):
        _validate_register(reg_or_imm)
        self.reg = reg_or_imm
        self.immediate = immediate

    def encode(self) -> bytes:
        if self.immediate:
            return bytes([0x3D, 0x80 + self.reg])  # ALT1 + MULT
        return bytes([0x80 + self.reg])


class UMULT(Instruction):
    """UMULT Rn - Dreg = Sreg * Rn (unsigned 8x8). ALT2/ALT3 variant of MULT."""

    def __init__(self, reg_or_imm: int, immediate: bool = False):
        _validate_register(reg_or_imm)
        self.reg = reg_or_imm
        self.immediate = immediate

    def encode(self) -> bytes:
        if self.immediate:
            return bytes([0x3F, 0x80 + self.reg])  # ALT3 + MULT
        return bytes([0x3E, 0x80 + self.reg])  # ALT2 + MULT


class FMULT(Instruction):
    """FMULT - Dreg = (Sreg * R6) >> 16 (fractional)."""

    def encode(self) -> bytes:
        return bytes([0x9F])


class LMULT(Instruction):
    """LMULT - R4:Dreg = Sreg * R6 (16x16 signed). ALT1 variant of FMULT."""

    def encode(self) -> bytes:
        return bytes([0x3D, 0x9F])  # ALT1 + FMULT


class INC(Instruction):
    """INC Rn - Rn = Rn + 1."""

    def __init__(self, reg: int):
        _validate_register(reg, 0, 14, "INC register")  # R15 is PC
        self.reg = reg

    def encode(self) -> bytes:
        return bytes([0xD0 + self.reg])


class DEC(Instruction):
    """DEC Rn - Rn = Rn - 1."""

    def __init__(self, reg: int):
        _validate_register(reg, 0, 14, "DEC register")  # R15 is PC
        self.reg = reg

    def encode(self) -> bytes:
        return bytes([0xE0 + self.reg])


# =============================================================================
# Logical Instructions
# =============================================================================

class AND(Instruction):
    """AND Rn - Dreg = Sreg AND Rn. With immediate=True: AND #n."""

    def __init__(self, reg_or_imm: int, immediate: bool = False):
        _validate_register(reg_or_imm, 1 if not immediate else 0, 15)
        self.reg = reg_or_imm
        self.immediate = immediate

    def encode(self) -> bytes:
        if self.immediate:
            return bytes([0x3D, 0x70 + self.reg])  # ALT1 + AND
        return bytes([0x70 + self.reg])


class OR(Instruction):
    """OR Rn - Dreg = Sreg OR Rn. With immediate=True: OR #n."""

    def __init__(self, reg_or_imm: int, immediate: bool = False):
        _validate_register(reg_or_imm, 1 if not immediate else 0, 15)
        self.reg = reg_or_imm
        self.immediate = immediate

    def encode(self) -> bytes:
        if self.immediate:
            return bytes([0x3D, 0xC0 + self.reg])  # ALT1 + OR
        return bytes([0xC0 + self.reg])


class XOR(Instruction):
    """XOR Rn - Dreg = Sreg XOR Rn. ALT2/ALT3 variant of OR."""

    def __init__(self, reg_or_imm: int, immediate: bool = False):
        _validate_register(reg_or_imm, 1 if not immediate else 0, 15)
        self.reg = reg_or_imm
        self.immediate = immediate

    def encode(self) -> bytes:
        if self.immediate:
            return bytes([0x3F, 0xC0 + self.reg])  # ALT3 + OR
        return bytes([0x3E, 0xC0 + self.reg])  # ALT2 + OR


class BIC(Instruction):
    """BIC Rn - Dreg = Sreg AND NOT Rn. ALT2/ALT3 variant of AND."""

    def __init__(self, reg_or_imm: int, immediate: bool = False):
        _validate_register(reg_or_imm, 1 if not immediate else 0, 15)
        self.reg = reg_or_imm
        self.immediate = immediate

    def encode(self) -> bytes:
        if self.immediate:
            return bytes([0x3F, 0x70 + self.reg])  # ALT3 + AND
        return bytes([0x3E, 0x70 + self.reg])  # ALT2 + AND


# =============================================================================
# Immediate Load Instructions
# =============================================================================

class IBT(Instruction):
    """IBT Rn, #xx - Load 8-bit signed immediate."""

    def __init__(self, reg: int, value: int):
        _validate_register(reg)
        self.reg = reg
        self.value = _to_signed_byte(value)

    def encode(self) -> bytes:
        return bytes([0xA0 + self.reg, self.value])


class IWT(Instruction):
    """IWT Rn, #xxxx - Load 16-bit immediate."""

    def __init__(self, reg: int, value: int):
        _validate_register(reg)
        self.reg = reg
        self.value = value & 0xFFFF

    def encode(self) -> bytes:
        lo = self.value & 0xFF
        hi = (self.value >> 8) & 0xFF
        return bytes([0xF0 + self.reg, lo, hi])


# =============================================================================
# ROM Access Instructions
# =============================================================================

class GETB(Instruction):
    """GETB - Get byte from ROM buffer to Dreg."""

    def encode(self) -> bytes:
        return bytes([0xEF])


class GETBH(Instruction):
    """GETBH - Get byte to high byte of Dreg. ALT1 variant of GETB."""

    def encode(self) -> bytes:
        return bytes([0x3D, 0xEF])  # ALT1 + GETB


class GETBL(Instruction):
    """GETBL - Get byte to low byte of Dreg. ALT2 variant of GETB."""

    def encode(self) -> bytes:
        return bytes([0x3E, 0xEF])  # ALT2 + GETB


class GETBS(Instruction):
    """GETBS - Get signed byte (sign-extended). ALT3 variant of GETB."""

    def encode(self) -> bytes:
        return bytes([0x3F, 0xEF])  # ALT3 + GETB


class GETC(Instruction):
    """GETC - Get byte to Color register."""

    def encode(self) -> bytes:
        return bytes([0xDF])


class ROMB(Instruction):
    """ROMB - Set ROM bank from Sreg. ALT2 variant of GETC."""

    def encode(self) -> bytes:
        return bytes([0x3E, 0xDF])  # ALT2 + GETC


class RAMB(Instruction):
    """RAMB - Set RAM bank from Sreg. ALT1 variant of GETC."""

    def encode(self) -> bytes:
        return bytes([0x3D, 0xDF])  # ALT1 + GETC


# =============================================================================
# Jump Instructions
# =============================================================================

class JMP(Instruction):
    """JMP Rn - Jump to address in Rn. Valid for R8-R13."""

    def __init__(self, reg: int):
        _validate_register(reg, 8, 13, "JMP register")
        self.reg = reg

    def encode(self) -> bytes:
        return bytes([0x98 + (self.reg - 8)])


class LJMP(Instruction):
    """LJMP Rn - Long jump (also sets PBR from Sreg). ALT1 variant of JMP."""

    def __init__(self, reg: int):
        _validate_register(reg, 8, 13, "LJMP register")
        self.reg = reg

    def encode(self) -> bytes:
        return bytes([0x3D, 0x98 + (self.reg - 8)])  # ALT1 + JMP


class LINK(Instruction):
    """LINK #n - R11 = R15 + n (save return address). n is 1-4."""

    def __init__(self, n: int):
        if not isinstance(n, int) or n < 1 or n > 4:
            raise ValueError(f"LINK value must be between 1 and 4, got {n}")
        self.n = n

    def encode(self) -> bytes:
        return bytes([0x90 + self.n])


# =============================================================================
# Helper function for instruction encoding
# =============================================================================

# Map of instruction names to classes
_INSTRUCTION_MAP = {
    'STOP': (STOP, 0),
    'NOP': (NOP, 0),
    'CACHE': (CACHE, 0),
    'LOOP': (LOOP, 0),
    'LSR': (LSR, 0),
    'ROL': (ROL, 0),
    'ASR': (ASR, 0),
    'ROR': (ROR, 0),
    'DIV2': (DIV2, 0),
    'PLOT': (PLOT, 0),
    'RPIX': (RPIX, 0),
    'COLOR': (COLOR, 0),
    'CMODE': (CMODE, 0),
    'SWAP': (SWAP, 0),
    'NOT': (NOT, 0),
    'HIB': (HIB, 0),
    'LOB': (LOB, 0),
    'SEX': (SEX, 0),
    'MERGE': (MERGE, 0),
    'SBK': (SBK, 0),
    'FMULT': (FMULT, 0),
    'LMULT': (LMULT, 0),
    'GETB': (GETB, 0),
    'GETBH': (GETBH, 0),
    'GETBL': (GETBL, 0),
    'GETBS': (GETBS, 0),
    'GETC': (GETC, 0),
    'ROMB': (ROMB, 0),
    'RAMB': (RAMB, 0),
    'ALT1': (ALT1, 0),
    'ALT2': (ALT2, 0),
    'ALT3': (ALT3, 0),
    # Single register operand
    'TO': (TO, 1),
    'FROM': (FROM, 1),
    'WITH': (WITH, 1),
    'STW': (STW, 1),
    'LDW': (LDW, 1),
    'STB': (STB, 1),
    'LDB': (LDB, 1),
    'ADD': (ADD, 1),
    'SUB': (SUB, 1),
    'ADC': (ADC, 1),
    'SBC': (SBC, 1),
    'CMP': (CMP, 1),
    'MULT': (MULT, 1),
    'UMULT': (UMULT, 1),
    'INC': (INC, 1),
    'DEC': (DEC, 1),
    'AND': (AND, 1),
    'OR': (OR, 1),
    'XOR': (XOR, 1),
    'BIC': (BIC, 1),
    'JMP': (JMP, 1),
    'LJMP': (LJMP, 1),
    'LINK': (LINK, 1),
    # Branch with offset
    'BRA': (BRA, 1),
    'BGE': (BGE, 1),
    'BLT': (BLT, 1),
    'BNE': (BNE, 1),
    'BEQ': (BEQ, 1),
    'BPL': (BPL, 1),
    'BMI': (BMI, 1),
    'BCC': (BCC, 1),
    'BCS': (BCS, 1),
    'BVC': (BVC, 1),
    'BVS': (BVS, 1),
    # Two arguments: register + value
    'IBT': (IBT, 2),
    'IWT': (IWT, 2),
    'LMS': (LMS, 2),
    'SMS': (SMS, 2),
    'LM': (LM, 2),
    'SM': (SM, 2),
}


def encode_instruction(name: str, *args) -> bytes:
    """Encode an instruction by name and arguments.

    Args:
        name: Instruction mnemonic (e.g., 'STOP', 'IBT', 'ADD')
        *args: Arguments for the instruction

    Returns:
        Encoded bytes for the instruction

    Raises:
        ValueError: If instruction name is unknown or wrong number of arguments
    """
    name_upper = name.upper()
    if name_upper not in _INSTRUCTION_MAP:
        raise ValueError(f"Unknown instruction: {name}")

    instr_class, num_args = _INSTRUCTION_MAP[name_upper]

    if len(args) != num_args:
        raise ValueError(f"{name} expects {num_args} argument(s), got {len(args)}")

    if num_args == 0:
        return instr_class().encode()
    elif num_args == 1:
        return instr_class(args[0]).encode()
    else:
        return instr_class(*args).encode()
