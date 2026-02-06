; SNES C Runtime Startup
; Initializes hardware and calls main()

.p816
.smart

; Import main function from LLVM-compiled code
.import main

; Import font data from font.s
.import font_data, font_data_size

; Import sprite data from sprites.s
.import sprite_data, sprite_data_size

; Import parallax data (optional - for parallax demo)
.import parallax_palette, parallax_palette_end
.import mountain_tiles, mountain_tiles_end
.import hill_tiles, hill_tiles_end
.import mountain_tilemap, mountain_tilemap_end
.import hill_tilemap, hill_tilemap_end
.import player_palette, player_palette_end

; Export entry point
.export reset, nmi_handler, irq_handler

.segment "STARTUP"

reset:
    sei                     ; Disable interrupts
    clc
    xce                     ; Switch to native mode
    rep #$30                ; 16-bit A/X/Y
    .a16
    .i16

    ; Initialize stack
    ldx #$1FFF
    txs

    ; Initialize Direct Page to 0
    lda #$0000
    tcd

    ; Clear PPU registers
    sep #$20                ; 8-bit A
    .a8

    lda #$80
    sta $2100               ; Force blank (screen off)

    stz $4200               ; Disable NMI/IRQ
    stz $420B               ; Disable DMA
    stz $420C               ; Disable HDMA

    ; Clear VRAM
    lda #$80
    sta $2115               ; VRAM increment mode

    rep #$20                ; 16-bit A
    .a16
    lda #$0000
    sta $2116               ; VRAM address = 0

    ldx #$0000
@clear_vram:
    stz $2118               ; Write 0 to VRAM
    stz $2118
    inx
    cpx #$4000              ; 32KB
    bne @clear_vram

    ; Clear CGRAM (palette)
    sep #$20
    .a8
    stz $2121               ; CGRAM address = 0
    ldx #$0200              ; 512 bytes
@clear_cgram:
    stz $2122
    dex
    bne @clear_cgram

    ; Clear OAM - set all sprites offscreen (Y = $F0)
    stz $2102               ; OAM address low
    stz $2103               ; OAM address high
    ldx #$0080              ; 128 sprites
@clear_oam:
    stz $2104               ; X = 0
    lda #$F0
    sta $2104               ; Y = 240 (offscreen)
    stz $2104               ; Tile = 0
    stz $2104               ; Attr = 0
    dex
    bne @clear_oam
    ; Clear high table (32 bytes)
    ldx #$0020
@clear_oam_high:
    stz $2104
    dex
    bne @clear_oam_high

    ; === GRAPHICS MODE SETUP ===
    ; Set Mode 0 (4 backgrounds, 2bpp each)
    lda #$00
    sta $2105               ; BGMODE = 0

    ; BG1 tilemap at VRAM $1000, 32x32 tiles
    lda #$10                ; ($1000 >> 9) | size=0
    sta $2107               ; BG1SC

    ; BG2 tilemap at VRAM $1800, 32x32 tiles
    lda #$18                ; ($1800 >> 9) | size=0
    sta $2108               ; BG2SC

    ; BG1 character data at VRAM $0000, BG2 at $2000
    ; Low nibble = BG1 base >> 12, High nibble = BG2 base >> 12
    lda #$20                ; BG1=$0000>>12=0, BG2=$2000>>12=2
    sta $210B               ; BG12NBA

    ; BG3 tilemap at VRAM $1C00, 32x32 tiles (for menu text overlay)
    lda #$1C                ; ($1C00 >> 9) | size=0
    sta $2109               ; BG3SC

    ; BG3 character data at VRAM $4000, BG4 unused
    ; Low nibble = BG3 base >> 12, High nibble = BG4 base >> 12
    lda #$04                ; BG3=$4000>>12=4, BG4=0
    sta $210C               ; BG34NBA

    ; Load font tiles to VRAM $4000 (for BG3 text)
    jsr load_font

    ; Load parallax tiles and tilemaps
    jsr load_mountain_tiles     ; BG1 tiles at $0000
    jsr load_hill_tiles         ; BG2 tiles at $2000
    jsr load_mountain_tilemap   ; BG1 tilemap at $1000
    jsr load_hill_tilemap       ; BG2 tilemap at $1800

    ; Load menu text to BG3 tilemap at $1C00
    jsr load_menu_tilemap

    ; Load parallax palettes
    ; BG1 uses CGRAM 0-3, BG2 uses CGRAM 32-35
    jsr load_parallax_palette
    jsr load_bg2_palette

    ; Load text palette for BG3 (palette 2, CGRAM colors 64-67)
    jsr load_text_palette

    ; === SPRITE SETUP ===
    ; OBSEL: sprite size and base address
    lda #$03                ; Base 3 = VRAM word $6000, size=0 (8x8/16x16)
    sta $2101               ; OBSEL

    ; Load player sprite palette (sprite palette 0, colors 128-143)
    jsr load_player_palette

    ; Enable BG1, BG2, BG3 and sprites on main screen
    lda #$17                ; Bit 0 = BG1, Bit 1 = BG2, Bit 2 = BG3, Bit 4 = OBJ
    sta $212C               ; TM = BG1 + BG2 + BG3 + OBJ enabled

    ; Load sprite graphics to VRAM
    jsr load_sprites

    ; Set up registers for 16-bit mode
    rep #$30                ; 16-bit A/X/Y
    .a16
    .i16

    ; Call main function
    jsr main

    ; Main returned - enter infinite loop
forever:
    wai                     ; Wait for interrupt
    bra forever


; Load font data into VRAM at $4000 (for BG3 text)
; Called with 8-bit A mode
load_font:
    php                     ; Save processor status
    rep #$20                ; 16-bit A
    .a16

    ; Set VRAM address to $4000
    lda #$4000
    sta $2116               ; VMADDL/VMADDH

    ; Set up for word writes, increment on high byte write
    sep #$20
    .a8
    lda #$80
    sta $2115               ; VMAIN

    rep #$20
    .a16

    ; Copy font data to VRAM
    ; Font is in RODATA segment, accessible via long addressing
    ldx #0
@font_loop:
    lda font_data,x
    sta $2118               ; VMDATAL/VMDATAH (16-bit write)
    inx
    inx
    cpx #font_data_size
    bcc @font_loop

    plp                     ; Restore processor status
    rts


; Load menu tilemap into VRAM at $1C00 (BG3 tilemap)
; Text uses font tiles (ASCII - 32 = tile number)
; Palette 2 (bits 10-12 of tilemap entry) = $2000
load_menu_tilemap:
    php
    rep #$20
    .a16

    ; Set VRAM address to $1C00
    lda #$1C00
    sta $2116

    sep #$20
    .a8
    lda #$80
    sta $2115               ; VMAIN: increment on high byte

    rep #$20
    .a16

    ; Clear tilemap (32x32 = 1024 entries, 2 bytes each)
    ldx #0
@clear:
    stz $2118
    inx
    cpx #1024
    bcc @clear

    ; Now write menu text at specific positions
    ; Row 2, Col 4: "TECH DEMO"
    ; Tilemap address = $1C00 + (row * 32 + col) * 2
    ; Row 2, Col 4 = $1C00 + (2*32 + 4) = $1C00 + 68 = $1C44
    lda #$1C44
    sta $2116
    sep #$20
    .a8
    ; "TECH DEMO" - tile = ASCII - 32, palette 2 = $20 high byte
    lda #('T'-32)
    sta $2118
    lda #$20                ; Palette 2
    sta $2119
    lda #('E'-32)
    sta $2118
    lda #$20
    sta $2119
    lda #('C'-32)
    sta $2118
    lda #$20
    sta $2119
    lda #('H'-32)
    sta $2118
    lda #$20
    sta $2119
    lda #(' '-32)
    sta $2118
    lda #$20
    sta $2119
    lda #('D'-32)
    sta $2118
    lda #$20
    sta $2119
    lda #('E'-32)
    sta $2118
    lda #$20
    sta $2119
    lda #('M'-32)
    sta $2118
    lda #$20
    sta $2119
    lda #('O'-32)
    sta $2118
    lda #$20
    sta $2119

    ; Row 5, Col 4: "1. PARALLAX"
    rep #$20
    .a16
    lda #$1CA4              ; $1C00 + (5*32 + 4) = $1C00 + 164 = $1CA4
    sta $2116
    sep #$20
    .a8
    lda #('1'-32)
    sta $2118
    lda #$20
    sta $2119
    lda #('.'-32)
    sta $2118
    lda #$20
    sta $2119
    lda #(' '-32)
    sta $2118
    lda #$20
    sta $2119
    lda #('P'-32)
    sta $2118
    lda #$20
    sta $2119
    lda #('A'-32)
    sta $2118
    lda #$20
    sta $2119
    lda #('R'-32)
    sta $2118
    lda #$20
    sta $2119
    lda #('A'-32)
    sta $2118
    lda #$20
    sta $2119
    lda #('L'-32)
    sta $2118
    lda #$20
    sta $2119
    lda #('L'-32)
    sta $2118
    lda #$20
    sta $2119
    lda #('A'-32)
    sta $2118
    lda #$20
    sta $2119
    lda #('X'-32)
    sta $2118
    lda #$20
    sta $2119

    ; Row 7, Col 4: "2. MOSAIC"
    rep #$20
    .a16
    lda #$1CE4              ; $1C00 + (7*32 + 4) = $1C00 + 228 = $1CE4
    sta $2116
    sep #$20
    .a8
    lda #('2'-32)
    sta $2118
    lda #$20
    sta $2119
    lda #('.'-32)
    sta $2118
    lda #$20
    sta $2119
    lda #(' '-32)
    sta $2118
    lda #$20
    sta $2119
    lda #('M'-32)
    sta $2118
    lda #$20
    sta $2119
    lda #('O'-32)
    sta $2118
    lda #$20
    sta $2119
    lda #('S'-32)
    sta $2118
    lda #$20
    sta $2119
    lda #('A'-32)
    sta $2118
    lda #$20
    sta $2119
    lda #('I'-32)
    sta $2118
    lda #$20
    sta $2119
    lda #('C'-32)
    sta $2118
    lda #$20
    sta $2119

    ; Row 9, Col 4: "3. INPUT"
    rep #$20
    .a16
    lda #$1D24              ; $1C00 + (9*32 + 4) = $1C00 + 292 = $1D24
    sta $2116
    sep #$20
    .a8
    lda #('3'-32)
    sta $2118
    lda #$20
    sta $2119
    lda #('.'-32)
    sta $2118
    lda #$20
    sta $2119
    lda #(' '-32)
    sta $2118
    lda #$20
    sta $2119
    lda #('I'-32)
    sta $2118
    lda #$20
    sta $2119
    lda #('N'-32)
    sta $2118
    lda #$20
    sta $2119
    lda #('P'-32)
    sta $2118
    lda #$20
    sta $2119
    lda #('U'-32)
    sta $2118
    lda #$20
    sta $2119
    lda #('T'-32)
    sta $2118
    lda #$20
    sta $2119

    ; Row 11, Col 4: "4. PALETTE"
    rep #$20
    .a16
    lda #$1D64              ; $1C00 + (11*32 + 4) = $1C00 + 356 = $1D64
    sta $2116
    sep #$20
    .a8
    lda #('4'-32)
    sta $2118
    lda #$20
    sta $2119
    lda #('.'-32)
    sta $2118
    lda #$20
    sta $2119
    lda #(' '-32)
    sta $2118
    lda #$20
    sta $2119
    lda #('P'-32)
    sta $2118
    lda #$20
    sta $2119
    lda #('A'-32)
    sta $2118
    lda #$20
    sta $2119
    lda #('L'-32)
    sta $2118
    lda #$20
    sta $2119
    lda #('E'-32)
    sta $2118
    lda #$20
    sta $2119
    lda #('T'-32)
    sta $2118
    lda #$20
    sta $2119
    lda #('T'-32)
    sta $2118
    lda #$20
    sta $2119
    lda #('E'-32)
    sta $2118
    lda #$20
    sta $2119

    ; Row 13, Col 4: "5. SPRITES"
    rep #$20
    .a16
    lda #$1DA4              ; $1C00 + (13*32 + 4) = $1C00 + 420 = $1DA4
    sta $2116
    sep #$20
    .a8
    lda #('5'-32)
    sta $2118
    lda #$20
    sta $2119
    lda #('.'-32)
    sta $2118
    lda #$20
    sta $2119
    lda #(' '-32)
    sta $2118
    lda #$20
    sta $2119
    lda #('S'-32)
    sta $2118
    lda #$20
    sta $2119
    lda #('P'-32)
    sta $2118
    lda #$20
    sta $2119
    lda #('R'-32)
    sta $2118
    lda #$20
    sta $2119
    lda #('I'-32)
    sta $2118
    lda #$20
    sta $2119
    lda #('T'-32)
    sta $2118
    lda #$20
    sta $2119
    lda #('E'-32)
    sta $2118
    lda #$20
    sta $2119
    lda #('S'-32)
    sta $2118
    lda #$20
    sta $2119

    plp
    rts


; Load text palette (palette 2 for BG3, CGRAM 64-67)
; Color 0 = transparent, Color 1 = white
load_text_palette:
    php
    sep #$20
    .a8

    lda #64                 ; BG3 palette 2 starts at CGRAM 64
    sta $2121

    ; Color 0: transparent (black, will be transparent due to color 0 rule)
    lda #$00
    sta $2122
    sta $2122

    ; Color 1: white ($7FFF = 15-bit white)
    lda #$FF
    sta $2122
    lda #$7F
    sta $2122

    ; Colors 2-3: also white for simplicity
    lda #$FF
    sta $2122
    lda #$7F
    sta $2122
    lda #$FF
    sta $2122
    lda #$7F
    sta $2122

    plp
    rts


; Load sprite data into VRAM at $6000 (OBSEL base 3)
; Called with 8-bit A mode
load_sprites:
    php                     ; Save processor status
    rep #$20                ; 16-bit A
    .a16

    ; Set VRAM address to $6000 (OBSEL base 3)
    lda #$6000
    sta $2116               ; VMADDL/VMADDH

    ; Set up for word writes, increment on high byte write
    sep #$20
    .a8
    lda #$80
    sta $2115               ; VMAIN

    rep #$20
    .a16

    ; Copy sprite data to VRAM
    ldx #0
@sprite_loop:
    lda sprite_data,x
    sta $2118               ; VMDATAL/VMDATAH (16-bit write)
    inx
    inx
    cpx #sprite_data_size
    bcc @sprite_loop

    plp                     ; Restore processor status
    rts


; Load mountain tiles into VRAM at $0000 (BG1 character data)
load_mountain_tiles:
    php
    rep #$20
    .a16

    lda #$0000
    sta $2116               ; VRAM address

    sep #$20
    .a8
    lda #$80
    sta $2115               ; VMAIN: increment on high byte

    rep #$20
    .a16

    ldx #0
@loop:
    lda mountain_tiles,x
    sta $2118
    inx
    inx
    cpx #(mountain_tiles_end - mountain_tiles)
    bcc @loop

    plp
    rts


; Load hill tiles into VRAM at $2000 (BG2 character data)
load_hill_tiles:
    php
    rep #$20
    .a16

    lda #$2000
    sta $2116               ; VRAM address

    sep #$20
    .a8
    lda #$80
    sta $2115               ; VMAIN

    rep #$20
    .a16

    ldx #0
@loop:
    lda hill_tiles,x
    sta $2118
    inx
    inx
    cpx #(hill_tiles_end - hill_tiles)
    bcc @loop

    plp
    rts


; Load mountain tilemap into VRAM at $1000 (BG1 tilemap)
load_mountain_tilemap:
    php
    rep #$20
    .a16

    lda #$1000
    sta $2116               ; VRAM address

    sep #$20
    .a8
    lda #$80
    sta $2115               ; VMAIN

    rep #$20
    .a16

    ldx #0
@loop:
    lda mountain_tilemap,x
    sta $2118
    inx
    inx
    cpx #(mountain_tilemap_end - mountain_tilemap)
    bcc @loop

    plp
    rts


; Load hill tilemap into VRAM at $1800 (BG2 tilemap)
load_hill_tilemap:
    php
    rep #$20
    .a16

    lda #$1800
    sta $2116               ; VRAM address

    sep #$20
    .a8
    lda #$80
    sta $2115               ; VMAIN

    rep #$20
    .a16

    ldx #0
@loop:
    lda hill_tilemap,x
    sta $2118
    inx
    inx
    cpx #(hill_tilemap_end - hill_tilemap)
    bcc @loop

    plp
    rts


; Load parallax palette to CGRAM (BG colors 0-15)
load_parallax_palette:
    php
    sep #$20
    .a8

    stz $2121               ; CGRAM address = 0

    ldx #0
@loop:
    lda parallax_palette,x
    sta $2122
    inx
    cpx #(parallax_palette_end - parallax_palette)
    bcc @loop

    plp
    rts


; Load BG2 palette to CGRAM 32-35 (Mode 0 BG2 uses this range)
load_bg2_palette:
    php
    sep #$20
    .a8

    lda #32                 ; BG2 palette starts at CGRAM 32
    sta $2121

    ; Load colors 4-7 from parallax_palette (the green hill colors)
    ; Each color is 2 bytes, so offset 8 bytes into parallax_palette
    ldx #0
@loop:
    lda parallax_palette+8,x   ; Start at color 4 (offset 8 bytes)
    sta $2122
    inx
    cpx #8                     ; 4 colors × 2 bytes = 8 bytes
    bcc @loop

    plp
    rts


; Load player sprite palette to CGRAM (sprite palette 0, colors 128-143)
load_player_palette:
    php
    sep #$20
    .a8

    lda #128                ; Sprite palettes start at CGRAM 128
    sta $2121

    ldx #0
@loop:
    lda player_palette,x
    sta $2122
    inx
    cpx #(player_palette_end - player_palette)
    bcc @loop

    plp
    rts


; NMI Handler (VBlank)
nmi_handler:
    rti

; IRQ Handler
irq_handler:
    rti


.segment "HEADER"
    ; SNES ROM Header at $FFB0
    .byte "  "              ; $FFB0-$FFB1: Maker code (2 bytes)
    .byte "DEMO"            ; $FFB2-$FFB5: Game code (4 bytes)
    .byte $00,$00,$00,$00,$00,$00,$00  ; $FFB6-$FFBC: Reserved (7 bytes)
    .byte $00               ; $FFBD: Expansion RAM size
    .byte $00               ; $FFBE: Special version
    .byte $00               ; $FFBF: Cartridge sub-type

    ; $FFC0-$FFD4: Game title (21 bytes, padded with spaces)
    .byte "LLVM W65816 DEMO"    ; 16 bytes
    .byte "     "               ; 5 spaces = 21 total

    .byte $20               ; $FFD5: Map mode (LoROM)
    .byte $00               ; $FFD6: Cartridge type (ROM only)
    .byte $05               ; $FFD7: ROM size (32KB = 2^5 * 1024)
    .byte $00               ; $FFD8: RAM size (0)
    .byte $01               ; $FFD9: Destination code (North America)
    .byte $00               ; $FFDA: Fixed value (old licensee)
    .byte $00               ; $FFDB: Version number
    .byte $00,$00           ; $FFDC-$FFDD: Checksum complement (filled later)
    .byte $00,$00           ; $FFDE-$FFDF: Checksum (filled later)


.segment "VECTORS"
    ; Native mode vectors ($FFE0-$FFEF)
    .word $0000             ; $FFE0: Reserved
    .word $0000             ; $FFE2: Reserved
    .word $0000             ; $FFE4: COP
    .word $0000             ; $FFE6: BRK
    .word $0000             ; $FFE8: ABORT
    .word nmi_handler       ; $FFEA: NMI
    .word $0000             ; $FFEC: Reserved
    .word irq_handler       ; $FFEE: IRQ

    ; Emulation mode vectors ($FFF0-$FFFF)
    .word $0000             ; $FFF0: Reserved
    .word $0000             ; $FFF2: Reserved
    .word $0000             ; $FFF4: COP
    .word $0000             ; $FFF6: Reserved
    .word $0000             ; $FFF8: ABORT
    .word nmi_handler       ; $FFFA: NMI
    .word reset             ; $FFFC: RESET
    .word irq_handler       ; $FFFE: IRQ/BRK
