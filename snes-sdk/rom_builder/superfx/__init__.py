"""SuperFX (GSU) Python Toolkit.

This package provides tools for building SuperFX ROMs and assembling GSU programs.

Example usage:

    from rom_builder.superfx import GSUAssembler, SuperFXROMBuilder

    # Assemble a simple GSU program
    asm = GSUAssembler()
    gsu_code = asm.assemble('''
        IBT R1, #0      ; X = 0
        IBT R2, #0      ; Y = 0
        IBT R0, #1      ; Color = 1
        COLOR           ; Set color register
        PLOT            ; Plot pixel
        STOP            ; Done
        NOP             ; Required after STOP
    ''')

    # Build a ROM containing the GSU program
    builder = SuperFXROMBuilder(gsu_code)
    builder.set_title("MY GAME")
    builder.set_screen_mode(128, 4)  # 128 height, 4 colors
    rom = builder.build()

    # Write to file
    with open('game.sfc', 'wb') as f:
        f.write(rom)
"""

# Assembler
from .gsu_assembler import GSUAssembler, AssemblyError

# ROM Builder
from .rom_builder import SuperFXROMBuilder

# Instruction classes (for programmatic assembly)
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
    # Helper
    encode_instruction,
)

# Register constants
from .gsu_registers import (
    # General registers
    R0, R1, R2, R3, R4, R5, R6, R7, R8, R9, R10, R11, R12, R13, R14, R15,
    # Control registers
    SFR, BRAMR, PBR, ROMBR, CFGR, SCBR, CLSR, SCMR, VCR, RAMBR, CBR,
    # SFR flags
    SFR_Z, SFR_CY, SFR_S, SFR_OV, SFR_G, SFR_R, SFR_ALT1, SFR_ALT2,
    SFR_IL, SFR_IH, SFR_B, SFR_IRQ,
    # SCMR constants
    SCMR_HT_128, SCMR_HT_160, SCMR_HT_192, SCMR_HT_OBJ,
    SCMR_MD_4COLOR, SCMR_MD_16COLOR, SCMR_MD_256COLOR,
    SCMR_RON, SCMR_RAN,
)

# Memory map utilities
from .memory_map import (
    snes_to_gsu_rom,
    snes_to_gsu_ram,
    gsu_to_snes_rom,
    gsu_to_snes_ram,
    is_rom_address,
    is_ram_address,
    is_register_address,
    get_screen_buffer_size,
    get_screen_base_address,
)

__all__ = [
    # Main classes
    'GSUAssembler',
    'SuperFXROMBuilder',
    'AssemblyError',
    # Instructions
    'STOP', 'NOP', 'CACHE', 'LOOP',
    'LSR', 'ROL', 'ASR', 'ROR', 'DIV2',
    'BRA', 'BGE', 'BLT', 'BNE', 'BEQ', 'BPL', 'BMI', 'BCC', 'BCS', 'BVC', 'BVS',
    'TO', 'FROM', 'WITH', 'ALT1', 'ALT2', 'ALT3',
    'STW', 'LDW', 'STB', 'LDB', 'SBK', 'LMS', 'SMS', 'LM', 'SM',
    'PLOT', 'RPIX', 'COLOR', 'CMODE',
    'SWAP', 'NOT', 'HIB', 'LOB', 'SEX', 'MERGE',
    'ADD', 'SUB', 'ADC', 'SBC', 'CMP', 'MULT', 'UMULT', 'FMULT', 'LMULT', 'INC', 'DEC',
    'AND', 'OR', 'XOR', 'BIC',
    'IBT', 'IWT',
    'GETB', 'GETBH', 'GETBL', 'GETBS', 'GETC', 'ROMB', 'RAMB',
    'JMP', 'LJMP', 'LINK',
    'encode_instruction',
    # Registers
    'R0', 'R1', 'R2', 'R3', 'R4', 'R5', 'R6', 'R7',
    'R8', 'R9', 'R10', 'R11', 'R12', 'R13', 'R14', 'R15',
    'SFR', 'BRAMR', 'PBR', 'ROMBR', 'CFGR', 'SCBR', 'CLSR', 'SCMR', 'VCR', 'RAMBR', 'CBR',
    'SFR_Z', 'SFR_CY', 'SFR_S', 'SFR_OV', 'SFR_G', 'SFR_R',
    'SFR_ALT1', 'SFR_ALT2', 'SFR_IL', 'SFR_IH', 'SFR_B', 'SFR_IRQ',
    'SCMR_HT_128', 'SCMR_HT_160', 'SCMR_HT_192', 'SCMR_HT_OBJ',
    'SCMR_MD_4COLOR', 'SCMR_MD_16COLOR', 'SCMR_MD_256COLOR',
    'SCMR_RON', 'SCMR_RAN',
    # Memory map
    'snes_to_gsu_rom', 'snes_to_gsu_ram', 'gsu_to_snes_rom', 'gsu_to_snes_ram',
    'is_rom_address', 'is_ram_address', 'is_register_address',
    'get_screen_buffer_size', 'get_screen_base_address',
]
