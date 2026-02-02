"""GSU (SuperFX) register constants and definitions.

This module defines all GSU registers with their SNES addresses,
SFR flag bit positions, and SCMR mode constants.
"""

# =============================================================================
# General Registers (R0-R15) - 16-bit registers at $3000-$301F
# =============================================================================

R0 = 0x3000   # Default source/destination
R1 = 0x3002   # PLOT X coordinate
R2 = 0x3004   # PLOT Y coordinate
R3 = 0x3006   # General purpose
R4 = 0x3008   # LMULT lower 16-bit result
R5 = 0x300A   # General purpose
R6 = 0x300C   # FMULT/LMULT multiplier
R7 = 0x300E   # MERGE source 1 (high byte)
R8 = 0x3010   # MERGE source 2 (low byte)
R9 = 0x3012   # General purpose
R10 = 0x3014  # General purpose
R11 = 0x3016  # LINK destination
R12 = 0x3018  # LOOP counter
R13 = 0x301A  # LOOP branch address
R14 = 0x301C  # ROM address pointer
R15 = 0x301E  # Program Counter (writing low byte starts GSU!)

# Register number to address mapping
REGISTER_ADDRESSES = {
    0: R0, 1: R1, 2: R2, 3: R3, 4: R4, 5: R5, 6: R6, 7: R7,
    8: R8, 9: R9, 10: R10, 11: R11, 12: R12, 13: R13, 14: R14, 15: R15
}

# =============================================================================
# Control Registers
# =============================================================================

SFR = 0x3030    # Status/Flag Register (16-bit, R/W)
BRAMR = 0x3033  # Backup RAM enable (1-bit, W)
PBR = 0x3034    # Program Bank Register (8-bit, R/W)
ROMBR = 0x3036  # ROM Bank Register (8-bit, R)
CFGR = 0x3037   # Config: IRQ mask, multiplier speed (8-bit, W)
SCBR = 0x3038   # Screen Base Register (8-bit, W)
CLSR = 0x3039   # Clock Select: 0=10MHz, 1=21MHz (1-bit, W)
SCMR = 0x303A   # Screen Mode Register (6-bit, W)
VCR = 0x303B    # Version Code Register (8-bit, R)
RAMBR = 0x303C  # RAM Bank Register: 0=bank $70, 1=bank $71 (1-bit, R)
CBR = 0x303E    # Cache Base Register (12-bit, R)

# =============================================================================
# SFR (Status/Flag Register) Bit Positions - $3030-$3031
#
# Layout:
#   $3031: [IRQ] [ B ] [IH ] [IL ] [ALT2] [ALT1] [ R ] [ G ]
#   $3030: [ - ] [ - ] [ - ] [ OV] [ S ] [CY ] [ Z ] [ - ]
# =============================================================================

# Low byte ($3030) flags
SFR_Z = 0x0002     # Zero flag (bit 1)
SFR_CY = 0x0004    # Carry flag (bit 2)
SFR_S = 0x0008     # Sign flag (bit 3)
SFR_OV = 0x0010    # Overflow flag (bit 4)

# High byte ($3031) flags (shifted to 16-bit position)
SFR_G = 0x0020     # Go flag (bit 5) - 1 = GSU running
SFR_R = 0x0040     # ROM Read flag (bit 6) - 1 = ROM buffer read in progress
SFR_ALT1 = 0x0100  # Alt Mode 1 (bit 8)
SFR_ALT2 = 0x0200  # Alt Mode 2 (bit 9)
SFR_IL = 0x0400    # Immediate Low pending (bit 10)
SFR_IH = 0x0800    # Immediate High pending (bit 11)
SFR_B = 0x1000     # B flag (bit 12) - set when WITH executed
SFR_IRQ = 0x8000   # Interrupt flag (bit 15) - set by STOP

# =============================================================================
# SCMR (Screen Mode Register) Constants - $303A
#
# Layout:
#   [ - ] [ - ] [HT1] [RON] [RAN] [HT0] [MD1] [MD0]
# =============================================================================

# Screen Height (HT1:HT0)
# Note: HT1 is bit 5, HT0 is bit 2
SCMR_HT_128 = 0x00  # 128 pixels (HT1:HT0 = 00)
SCMR_HT_160 = 0x04  # 160 pixels (HT1:HT0 = 01) - HT0 is bit 2
SCMR_HT_192 = 0x20  # 192 pixels (HT1:HT0 = 10) - HT1 is bit 5
SCMR_HT_OBJ = 0x24  # OBJ mode (HT1:HT0 = 11)

# Color Depth (MD1:MD0)
SCMR_MD_4COLOR = 0x00   # 4 colors / 2bpp (MD1:MD0 = 00)
SCMR_MD_16COLOR = 0x01  # 16 colors / 4bpp (MD1:MD0 = 01)
SCMR_MD_256COLOR = 0x03 # 256 colors / 8bpp (MD1:MD0 = 11)

# Bus Control
SCMR_RAN = 0x08  # RAM bus: 0 = SNES owns, 1 = GSU owns (bit 3)
SCMR_RON = 0x10  # ROM bus: 0 = SNES owns, 1 = GSU owns (bit 4)

# =============================================================================
# Register Size
# =============================================================================

REGISTER_SIZE = 2  # All GSU registers are 16-bit (2 bytes)
