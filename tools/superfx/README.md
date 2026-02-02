# SuperFX Python Toolkit

A Python toolkit for building SuperFX (GSU) ROMs and assembling GSU programs for the Super Nintendo.

## Overview

This toolkit provides:
- **GSUAssembler**: Assembles GSU assembly language to machine code
- **SuperFXROMBuilder**: Creates complete SuperFX ROM files with proper headers
- **Instruction classes**: Programmatic assembly of GSU instructions
- **Register constants**: All GSU registers and flags
- **Memory map utilities**: Address translation and screen buffer calculations

## Installation

The toolkit is part of the llvm-experiments project. No additional dependencies required beyond Python 3.8+.

## Quick Start

```python
from tools.superfx import GSUAssembler, SuperFXROMBuilder

# Assemble a simple GSU program
asm = GSUAssembler()
gsu_code = asm.assemble('''
    IBT R0, #1      ; Color = 1 (red)
    COLOR           ; Set color register
    IBT R1, #0      ; X = 0
    IBT R2, #0      ; Y = 0
    PLOT            ; Plot pixel at (0, 0)
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
```

## GSU Assembly Syntax

### Registers
- `R0` - `R15`: General purpose registers
- `R15` is the program counter (PC)
- `R0` is the default source/destination register

### Supported Instructions

**Control:**
- `STOP` - Stop GSU execution
- `NOP` - No operation
- `CACHE` - Set cache base
- `LOOP` - Decrement R12, branch to R13 if != 0

**Arithmetic:**
- `ADD Rn` / `ADD #n` - Add
- `SUB Rn` / `SUB #n` - Subtract
- `ADC Rn` - Add with carry
- `SBC Rn` - Subtract with carry
- `CMP Rn` - Compare (flags only)
- `INC Rn` - Increment
- `DEC Rn` - Decrement
- `MULT Rn` / `MULT #n` - Multiply (8x8 signed)
- `UMULT Rn` - Multiply (8x8 unsigned)

**Logical:**
- `AND Rn` / `AND #n` - Logical AND
- `OR Rn` / `OR #n` - Logical OR
- `XOR Rn` / `XOR #n` - Logical XOR
- `BIC Rn` - Bit clear (AND NOT)
- `NOT` - Logical NOT

**Shifts:**
- `LSR` - Logical shift right
- `ASR` - Arithmetic shift right
- `ROL` - Rotate left through carry
- `ROR` - Rotate right through carry

**Memory:**
- `LDW (Rn)` - Load word from RAM
- `STW (Rn)` - Store word to RAM
- `LDB (Rn)` - Load byte from RAM
- `STB (Rn)` - Store byte to RAM
- `LM Rn, (addr)` - Load from address
- `SM (addr), Rn` - Store to address

**Immediate Loads:**
- `IBT Rn, #xx` - Load 8-bit immediate (sign-extended)
- `IWT Rn, #xxxx` - Load 16-bit immediate

**Plotting:**
- `PLOT` - Plot pixel at (R1, R2) with current color
- `RPIX` - Read pixel at (R1, R2)
- `COLOR` - Set color from R0
- `CMODE` - Set plot mode

**Branches:**
- `BRA label` - Branch always
- `BEQ label` - Branch if equal (Z=1)
- `BNE label` - Branch if not equal (Z=0)
- `BPL label` - Branch if plus (S=0)
- `BMI label` - Branch if minus (S=1)
- `BCC label` - Branch if carry clear
- `BCS label` - Branch if carry set
- `BGE label` - Branch if >= (signed)
- `BLT label` - Branch if < (signed)

**Register Prefixes:**
- `TO Rn` - Set destination register
- `FROM Rn` - Set source register
- `WITH Rn` - Set both source and destination

**Jumps:**
- `JMP Rn` - Jump to address in Rn (R8-R13)
- `LJMP Rn` - Long jump with bank
- `LINK #n` - Save return address (n=1-4)

### Numeric Literals

```asm
IBT R0, #42       ; Decimal
IBT R0, #$2A      ; Hexadecimal
IBT R0, #%101010  ; Binary
```

### Labels

```asm
loop:
    PLOT
    INC R1
    BNE loop
    NOP
```

## Register Constants

```python
from tools.superfx.gsu_registers import (
    R0, R1, R2, R3, R4, R5, R6, R7,
    R8, R9, R10, R11, R12, R13, R14, R15,
    SFR, SCMR, SCBR, PBR,
    SCMR_HT_128, SCMR_MD_4COLOR, SCMR_RON, SCMR_RAN,
)
```

## Programmatic Assembly

```python
from tools.superfx import IBT, COLOR, PLOT, STOP, NOP

# Build instructions programmatically
instructions = [
    IBT(0, 1),      # R0 = 1
    COLOR(),        # Set color
    PLOT(),         # Plot pixel
    STOP(),
    NOP(),
]

code = b''.join(instr.encode() for instr in instructions)
```

## Memory Map Utilities

```python
from tools.superfx.memory_map import (
    get_screen_buffer_size,
    snes_to_gsu_rom,
    gsu_to_snes_ram,
)

# Calculate screen buffer size for 128-height, 4-color mode
size = get_screen_buffer_size(128, 4)  # Returns 8192 bytes
```

## Testing

```bash
# Run all toolkit tests
python -m pytest tools/superfx/tests/ -v

# 251 tests covering:
# - Register constants (40 tests)
# - Instruction encoding (113 tests)
# - Assembler (48 tests)
# - Memory map (26 tests)
# - ROM builder (24 tests)
```

## Known Limitations

### Emulator Compatibility

**Important:** Homebrew SuperFX ROMs may not work correctly on all emulators.

During testing, we discovered that both **Snes9x** and **Ares** have issues with homebrew SuperFX ROMs:
- The GSU executes correctly (verified via diagnostics)
- But the SNES CPU cannot read GSU RAM at bank $70
- DMA from $70 returns uninitialized data ($FF or $00)
- Commercial games like Star Fox work fine on these emulators

This appears to be an emulator limitation where GSU RAM is not properly initialized for ROMs that aren't in the emulator's game database.

**Workarounds:**
1. Test on **bsnes/higan** (most accurate SNES emulator)
2. Test on **real hardware** with a flash cart (e.g., SD2SNES/FXPak)
3. Use direct VRAM writes for display testing (bypasses GSU RAM)

### GSU Instruction Coverage

The assembler supports all common GSU instructions. Some rarely-used variants may not be implemented.

## References

- [SuperFX Programming Reference](../../docs/superfx-reference.md)
- [GSU Instruction Set](https://wiki.superfamicom.org/gsu)
- [SNES Development Wiki](https://wiki.superfamicom.org/)

## License

Part of the llvm-experiments project.
