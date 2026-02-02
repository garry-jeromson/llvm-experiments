# SuperFX (GSU) Technical Reference

This document provides a comprehensive reference for the SuperFX coprocessor (Graphics Support Unit) used in select SNES cartridges.

## Overview

The SuperFX is a 16-bit RISC CPU designed by Argonaut Games for Nintendo. It was originally developed for Star Fox and later used in games like Yoshi's Island, Doom, and Stunt Race FX.

**Key Features:**
- 16-bit RISC architecture with 16 general-purpose registers
- Clock speed: 10.74 MHz (GSU-1) or 21.47 MHz (GSU-2 high-speed mode)
- 512-byte instruction cache for single-cycle execution
- Built-in pixel plotting hardware with automatic bitplane conversion
- Independent ROM and RAM buses for parallel access

## Chip Variants

| Variant | Max ROM | Clock Speed | Notable Games |
|---------|---------|-------------|---------------|
| MARIO Chip | 1MB | 10 MHz | Star Fox |
| GSU-1 | 1MB | 10/21.47 MHz | Stunt Race FX, Dirt Trax FX |
| GSU-2 | 2MB | 10/21.47 MHz | Yoshi's Island, Doom, Winter Gold |

---

## Register Set

### General Registers (R0-R15)

All registers are 16-bit. R0-R13 are general purpose with some having special functions for certain instructions.

| Register | SNES Address | Special Function | Initial Value |
|----------|--------------|------------------|---------------|
| R0 | $3000-$3001 | Default source/destination | Invalid |
| R1 | $3002-$3003 | PLOT X coordinate | $0000 |
| R2 | $3004-$3005 | PLOT Y coordinate | $0000 |
| R3 | $3006-$3007 | (none) | Invalid |
| R4 | $3008-$3009 | LMULT lower 16-bit result | Invalid |
| R5 | $300A-$300B | (none) | Invalid |
| R6 | $300C-$300D | FMULT/LMULT multiplier | Invalid |
| R7 | $300E-$300F | MERGE source 1 (high byte) | Invalid |
| R8 | $3010-$3011 | MERGE source 2 (low byte) | Invalid |
| R9 | $3012-$3013 | (none) | Invalid |
| R10 | $3014-$3015 | (none) | Invalid |
| R11 | $3016-$3017 | LINK destination | Invalid |
| R12 | $3018-$3019 | LOOP counter | Invalid |
| R13 | $301A-$301B | LOOP branch address | Invalid |
| R14 | $301C-$301D | ROM address pointer | Invalid |
| R15 | $301E-$301F | Program Counter | $0000 |

**Important:** Writing to $301E (R15 low byte) from the SNES CPU starts GSU execution.

### Control Registers

| Register | Address | Size | Access | Description |
|----------|---------|------|--------|-------------|
| SFR | $3030-$3031 | 16-bit | R/W | Status/Flag Register |
| BRAMR | $3033 | 1-bit | W | Backup RAM enable |
| PBR | $3034 | 8-bit | R/W | Program Bank Register |
| ROMBR | $3036 | 8-bit | R | ROM Bank Register |
| CFGR | $3037 | 8-bit | W | Config (IRQ mask, multiplier speed) |
| SCBR | $3038 | 8-bit | W | Screen Base Register |
| CLSR | $3039 | 1-bit | W | Clock Select (0=10MHz, 1=21MHz) |
| SCMR | $303A | 6-bit | W | Screen Mode Register |
| VCR | $303B | 8-bit | R | Version Code Register |
| RAMBR | $303C | 1-bit | R | RAM Bank Register (0=bank $70, 1=bank $71) |
| CBR | $303E-$303F | 12-bit | R | Cache Base Register |

All registers are accessible in banks $00-$3F and $80-$BF.

### Status/Flag Register (SFR) - $3030-$3031

```
$3031: [IRQ] [ B ] [IH ] [IL ] [ALT2] [ALT1] [ R ] [ G ]
$3030: [ - ] [ - ] [ - ] [ OV] [ S ] [CY ] [ Z ] [ - ]
```

| Bit | Name | Description |
|-----|------|-------------|
| Z | Zero | Set when result is zero |
| CY | Carry | Carry/borrow flag |
| S | Sign | Set when result is negative (bit 15 = 1) |
| OV | Overflow | Signed overflow |
| G | Go | 1 = GSU is running, 0 = GSU stopped |
| R | ROM Read | 1 = ROM buffer read in progress |
| ALT1 | Alt Mode 1 | Prefix flag for next instruction |
| ALT2 | Alt Mode 2 | Prefix flag for next instruction |
| IL | Immediate Low | Immediate byte pending (low) |
| IH | Immediate High | Immediate byte pending (high) |
| B | B Flag | Set when WITH instruction executed |
| IRQ | Interrupt | Interrupt request flag (set by STOP) |

### Screen Mode Register (SCMR) - $303A

```
[ - ] [ - ] [HT1] [RON] [RAN] [HT0] [MD1] [MD0]
```

**Screen Height (HT1:HT0):**
- 00 = 128 pixels
- 01 = 160 pixels
- 10 = 192 pixels
- 11 = OBJ mode (128x128 quadrants)

**Color Depth (MD1:MD0):**
- 00 = 4 colors (2bpp)
- 01 = 16 colors (4bpp)
- 10 = (not used)
- 11 = 256 colors (8bpp)

**Bus Control:**
- RON: 0 = SNES owns ROM bus, 1 = GSU owns ROM bus
- RAN: 0 = SNES owns RAM bus, 1 = GSU owns RAM bus

---

## Memory Map

### SNES CPU Perspective

| Address Range | Description |
|---------------|-------------|
| $00-$3F:$3000-$32FF | GSU registers and cache |
| $00-$3F:$6000-$7FFF | Game Pak RAM mirror (8KB) |
| $00-$3F:$8000-$FFFF | Game Pak ROM (LoROM) |
| $40-$5F:$0000-$FFFF | Game Pak ROM (HiROM mirror) |
| $70-$71:$0000-$FFFF | Game Pak RAM (128KB total) |
| $78-$79:$0000-$FFFF | Backup RAM (128KB) |
| $80-$BF:$3000-$32FF | GSU registers mirror |
| $80-$BF:$8000-$FFFF | Game Pak ROM mirror |

### GSU Perspective

| Address Range | Description |
|---------------|-------------|
| $00-$3F:$8000-$FFFF | Game Pak ROM (LoROM, 32KB per bank) |
| $40-$5F:$0000-$FFFF | Game Pak ROM (HiROM, 64KB per bank) |
| $70-$71:$0000-$FFFF | Game Pak RAM (64KB per bank) |

**Important Notes:**
- GSU uses PBR (Program Bank) for instruction fetch
- GSU uses ROMBR for ROM data reads via R14
- GSU uses RAMBR for RAM access (bank $70 or $71)
- The SNES CPU and GSU cannot access ROM/RAM simultaneously

---

## Instruction Set

The GSU has 98 instructions. All opcodes are 8-bit, but prefix opcodes (ALT1, ALT2, ALT3) modify the meaning of the following instruction.

### Instruction Categories

#### Data Transfer

| Instruction | Description |
|-------------|-------------|
| `MOVE Rn, Rm` | Copy Rm to Rn |
| `MOVES Rn, Rm` | Copy Rm to Rn, set flags |
| `IWT Rn, #xxxx` | Load 16-bit immediate |
| `IBT Rn, #xx` | Load 8-bit signed immediate |
| `LDW (Rm)` | Load word from RAM[Rm] |
| `LDB (Rm)` | Load byte from RAM[Rm] |
| `LM (xxxx)` | Load word from RAM address |
| `LMS (xx)` | Load word from RAM short address |
| `STW (Rm)` | Store word to RAM[Rm] |
| `STB (Rm)` | Store byte to RAM[Rm] |
| `SM (xxxx), Rn` | Store word to RAM address |
| `SMS (xx), Rn` | Store word to RAM short address |
| `SBK` | Store word to last RAM address |

#### ROM Access

| Instruction | Description |
|-------------|-------------|
| `GETB` | Get byte from ROM buffer to Dreg |
| `GETBH` | Get byte to high byte of Dreg |
| `GETBL` | Get byte to low byte of Dreg |
| `GETBS` | Get signed byte (sign-extended) |
| `GETC` | Get byte to Color register |
| `ROMB` | Set ROM bank from Sreg |

#### Arithmetic

| Instruction | ALT | Description |
|-------------|-----|-------------|
| `ADD Rn` | - | Dreg = Sreg + Rn |
| `ADD #n` | ALT1 | Dreg = Sreg + n (n=0-15) |
| `ADC Rn` | ALT2 | Dreg = Sreg + Rn + Carry |
| `ADI #n` | ALT3 | Dreg = Sreg + n + Carry |
| `SUB Rn` | - | Dreg = Sreg - Rn |
| `SUB #n` | ALT1 | Dreg = Sreg - n |
| `SBC Rn` | ALT2 | Dreg = Sreg - Rn - ~Carry |
| `CMP Rn` | ALT3 | Compare Sreg - Rn (flags only) |
| `MULT Rn` | - | Dreg = Sreg * Rn (signed 8x8) |
| `MULT #n` | ALT1 | Dreg = Sreg * n (signed) |
| `UMULT Rn` | ALT2 | Dreg = Sreg * Rn (unsigned 8x8) |
| `UMULT #n` | ALT3 | Dreg = Sreg * n (unsigned) |
| `FMULT` | - | Dreg = (Sreg * R6) >> 16 (fractional) |
| `LMULT` | ALT1 | R4:Dreg = Sreg * R6 (16x16 signed) |
| `DIV2` | - | Dreg = Sreg / 2 (arithmetic) |
| `INC Rn` | - | Rn = Rn + 1 |
| `DEC Rn` | - | Rn = Rn - 1 |

#### Logical

| Instruction | ALT | Description |
|-------------|-----|-------------|
| `AND Rn` | - | Dreg = Sreg AND Rn |
| `AND #n` | ALT1 | Dreg = Sreg AND n |
| `BIC Rn` | ALT2 | Dreg = Sreg AND NOT Rn |
| `BIC #n` | ALT3 | Dreg = Sreg AND NOT n |
| `OR Rn` | - | Dreg = Sreg OR Rn |
| `OR #n` | ALT1 | Dreg = Sreg OR n |
| `XOR Rn` | ALT2 | Dreg = Sreg XOR Rn |
| `XOR #n` | ALT3 | Dreg = Sreg XOR n |
| `NOT` | - | Dreg = NOT Sreg |

#### Shifts

| Instruction | Description |
|-------------|-------------|
| `ASR` | Arithmetic shift right (preserves sign) |
| `LSR` | Logical shift right |
| `ROL` | Rotate left through carry |
| `ROR` | Rotate right through carry |

#### Byte Operations

| Instruction | Description |
|-------------|-------------|
| `HIB` | Dreg = high byte of Sreg |
| `LOB` | Dreg = low byte of Sreg |
| `SWAP` | Swap high and low bytes |
| `SEX` | Sign extend byte to word |
| `MERGE` | Dreg = R7[15:8] : R8[15:8] |

#### Branching

| Instruction | Description |
|-------------|-------------|
| `BRA e` | Branch always (relative) |
| `BGE e` | Branch if >= 0 (signed) |
| `BLT e` | Branch if < 0 (signed) |
| `BNE e` | Branch if not equal (Z=0) |
| `BEQ e` | Branch if equal (Z=1) |
| `BPL e` | Branch if plus (S=0) |
| `BMI e` | Branch if minus (S=1) |
| `BCC e` | Branch if carry clear |
| `BCS e` | Branch if carry set |
| `BVC e` | Branch if overflow clear |
| `BVS e` | Branch if overflow set |
| `JMP Rn` | Jump to address in Rn |
| `LJMP Rn` | Long jump (also sets PBR from Sreg) |

#### Register Selection Prefixes

| Instruction | Description |
|-------------|-------------|
| `TO Rn` | Set destination register for next op |
| `FROM Rn` | Set source register for next op |
| `WITH Rn` | Set both source and destination |
| `ALT1` | Enable ALT1 mode for next instruction |
| `ALT2` | Enable ALT2 mode for next instruction |
| `ALT3` | Enable ALT1+ALT2 for next instruction |

#### Plotting

| Instruction | Description |
|-------------|-------------|
| `PLOT` | Plot pixel at (R1, R2) with COLR |
| `RPIX` | Read pixel at (R1, R2) to Dreg |
| `COLOR` | Set COLR from low byte of Sreg |
| `CMODE` | Set plot options from Sreg |

#### Control

| Instruction | Description |
|-------------|-------------|
| `STOP` | Stop GSU execution, set IRQ flag |
| `NOP` | No operation |
| `CACHE` | Set cache base, invalidate cache |
| `LOOP` | Decrement R12, branch to R13 if != 0 |
| `LINK #n` | R11 = R15 + n (save return address) |
| `RAMB` | Set RAM bank from Sreg |

---

## Instruction Encoding

### Opcode Matrices

The GSU uses four opcode matrices based on ALT1 and ALT2 prefix states:

- **ALT0** (ALT1=0, ALT2=0): Default instruction set
- **ALT1** (ALT1=1, ALT2=0): Immediate variants
- **ALT2** (ALT1=0, ALT2=1): Unsigned/alternate operations
- **ALT3** (ALT1=1, ALT2=1): Compare and bit-clear variants

### Common Opcodes (ALT0)

| Opcode | Instruction |
|--------|-------------|
| $00 | STOP |
| $01 | NOP |
| $02 | CACHE |
| $03 | LSR |
| $04 | ROL |
| $05 | BRA |
| $06 | BGE |
| $07 | BLT |
| $08 | BNE |
| $09 | BEQ |
| $0A | BPL |
| $0B | BMI |
| $0C | BCC |
| $0D | BCS |
| $0E | BVC |
| $0F | BVS |
| $10-$1F | TO R0-R15 |
| $20-$2F | WITH R0-R15 |
| $30-$3B | STW (R0-R11) |
| $3C | LOOP |
| $3D | ALT1 |
| $3E | ALT2 |
| $3F | ALT3 |
| $40-$4B | LDW (R0-R11) |
| $4C | PLOT / RPIX (ALT1) |
| $4D | SWAP |
| $4E | COLOR / CMODE (ALT1) |
| $4F | NOT |
| $50-$5F | ADD R0-R15 |
| $60-$6F | SUB R0-R15 |
| $70 | MERGE |
| $71-$7F | AND R1-R15 |
| $80-$8F | MULT R0-R15 |
| $90 | SBK |
| $91-$94 | LINK #1-4 |
| $95 | SEX |
| $96 | ASR / DIV2 (ALT1) |
| $97 | ROR |
| $98-$9D | JMP R8-R13 |
| $9E | LOB |
| $9F | FMULT / LMULT (ALT1) |
| $A0-$AF | IBT R0-R15, #xx / LMS (ALT1) / SMS (ALT2) |
| $B0-$BF | FROM R0-R15 |
| $C0-$CF | HIB / OR R0-R15 |
| $D0-$DE | INC R0-R14 |
| $DF | GETC / RAMB (ALT1) / ROMB (ALT2) |
| $E0-$EE | DEC R0-R14 |
| $EF | GETB / GETBH (ALT1) / GETBL (ALT2) / GETBS (ALT3) |
| $F0-$FF | IWT R0-R15, #xxxx / LM (ALT1) / SM (ALT2) |

---

## Execution Flow

### Starting the GSU

1. Ensure GSU is stopped (check G flag in SFR)
2. Set up program bank (write to PBR at $3034)
3. Set up ROM bank if needed (GSU will use ROMB instruction)
4. Set up RAM bank if needed (GSU will use RAMB instruction)
5. Configure screen mode (write to SCBR and SCMR)
6. Write program counter (write low byte to $301E, then high byte to $301F)
   - **Writing to $301E starts execution!**

```asm
; Example: Start GSU at address $8000 in bank $00
    lda #$00
    sta $3034       ; PBR = bank $00

    lda #$00
    sta $301F       ; R15 high = $00
    lda #$00
    sta $301E       ; R15 low = $00 (this starts GSU!)
```

### Stopping the GSU

The GSU executes until it encounters a `STOP` instruction, which:
1. Clears the G (Go) flag in SFR
2. Sets the IRQ flag
3. Optionally generates an interrupt to the SNES CPU (if not masked)

### Polling for Completion

```asm
wait_gsu:
    lda $3030       ; Read SFR low byte
    and #$20        ; Check G flag (bit 5)
    bne wait_gsu    ; Loop while GSU is running
```

**Important:** While the GSU is running, the SNES CPU should NOT access cartridge ROM/RAM, or it will receive garbage data. The CPU wait loop should be in WRAM.

---

## Code Cache

The GSU has a 512-byte code cache (organized as 32 x 16-byte blocks) for high-speed instruction execution.

### Cache Operation

- Instructions in cache execute in ~1 cycle
- Instructions from ROM/RAM take 3-5 cycles
- Cache is automatically filled during execution
- Manual cache control via `CACHE` and `LJMP` instructions

### CACHE Instruction

The `CACHE` instruction:
1. Invalidates all cached data
2. Sets CBR to current PC (aligned to 512 bytes)
3. Subsequent code will be cached from this base

### Cache Base Register (CBR)

- 12-bit register at $303E-$303F
- Specifies the base address for cached code
- Only bits 4-15 are used (16-byte alignment)

---

## Pixel Plotting

The GSU has specialized hardware for bitmap rendering that automatically converts to SNES bitplane format.

### Screen Configuration

Set up via SCBR (base address) and SCMR (mode):

| Mode | Resolution | Colors | Screen Base Usage |
|------|------------|--------|-------------------|
| 128H, 4-color | 256x128 | 4 | 4KB |
| 128H, 16-color | 256x128 | 16 | 8KB |
| 128H, 256-color | 256x128 | 256 | 16KB |
| 160H, 4-color | 256x160 | 4 | 5KB |
| 192H, 4-color | 256x192 | 4 | 6KB |
| OBJ mode | 4x 128x128 | varies | Sprite format |

### Plotting Process

1. Set color: `COLOR` instruction (loads COLR from Sreg low byte)
2. Set coordinates: Load R1 (X) and R2 (Y)
3. Plot: `PLOT` instruction

The PLOT instruction:
- Writes to an 8-pixel buffer
- Automatically flushes when 8 pixels written or coordinates change
- Converts bitmap to bitplane format during flush

### Plot Option Register (CMODE)

```
[ - ] [ - ] [ - ] [OBJ] [FHN] [FRZ] [DTH] [TRA]
```

| Bit | Name | Description |
|-----|------|-------------|
| TRA | Transparent | Don't plot if color = 0 |
| DTH | Dither | Dither using X^Y parity |
| FRZ | Freeze | Freeze high nibble |
| FHN | Force High Nibble | Duplicate low nibble to high |
| OBJ | Object Mode | Use sprite coordinate mode |

---

## ROM Buffer

The GSU uses an asynchronous ROM buffer for data reads:

1. Write address to R14 (triggers buffer reload)
2. Set bank with `ROMB` instruction if needed
3. Read data with `GETB`, `GETBH`, `GETBL`, or `GETBS`

The buffer reload happens in parallel with instruction execution. Instructions only stall if the buffer isn't ready when needed.

---

## RAM Buffer

RAM access also uses buffering:

- **Writes:** `STW`/`STB`/`SM`/`SMS`/`SBK` queue asynchronously
- **Reads:** `LDW`/`LDB`/`LM`/`LMS` block until data arrives
- `SBK` stores to the last-read address (useful for read-modify-write)
- `RPIX` (read pixel) flushes pixel buffer and waits for pending writes

---

## Pipeline Considerations

The GSU uses pipelining, which creates a **delay slot** after branches:

```asm
    BCC skip        ; Branch if carry clear
    SUB R0          ; This ALWAYS executes (delay slot)
skip:
    ...
```

The instruction after any branch always executes. Use `NOP` or a useful instruction in the delay slot.

**After STOP:**
```asm
    STOP            ; Stop execution
    NOP             ; Required - dummy opcode after STOP must be NOP
```

---

## SNES Header for SuperFX Cartridges

For a SuperFX cartridge, the ROM header at $FFD6 should be:

| Offset | Value | Description |
|--------|-------|-------------|
| $FFD5 | $20 | Map mode (LoROM) |
| $FFD6 | $13 | Cartridge type (SuperFX) |
| $FFD7 | $08+ | ROM size ($08 = 256KB minimum) |
| $FFD8 | $05 | RAM size ($05 = 64KB typical) |

**Note:** SuperFX cartridges require a minimum 256KB ROM.

---

## Example: Minimal GSU Program

```asm
; GSU code to fill screen with a color
; Assumes setup: 256x128, 4-color mode

    IBT R1, #0      ; X = 0
    IBT R2, #0      ; Y = 0
    IBT R0, #1      ; Color = 1
    COLOR           ; Set color register

fill_loop:
    PLOT            ; Plot pixel at (R1, R2)
    INC R1          ; X++
    IBT R3, #255    ; Compare value
    CMP R3          ; R1 == 255?
    BNE fill_loop   ; Continue row
    NOP             ; Delay slot

    IBT R1, #0      ; X = 0
    INC R2          ; Y++
    IBT R3, #127    ; Compare value
    CMP R3          ; R2 == 127?
    BNE fill_loop   ; Continue screen
    NOP             ; Delay slot

    STOP            ; Done
    NOP             ; Required after STOP
```

---

## References

- [SnesLab Super FX Wiki](https://sneslab.net/wiki/Super_FX) - Comprehensive technical documentation
- [SnesLab Opcode Matrices](https://sneslab.net/wiki/Super_FX_Opcode_Matrices) - Complete opcode tables
- [SNESdev Wiki - Super FX](https://snes.nesdev.org/wiki/Super_FX) - Additional reference
- [jsgroth's Blog - SNES Coprocessors](https://jsgroth.dev/blog/posts/snes-coprocessors-part-7/) - Emulation details
- Nintendo SNES Development Manual, Book II, Section 2 - Official documentation
