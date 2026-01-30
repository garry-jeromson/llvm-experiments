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

    ; BG1 tilemap at VRAM $1000 (address bits 15-10 = $04, 32x32 tiles)
    lda #$10                ; ($1000 >> 9) | size=0
    sta $2107               ; BG1SC

    ; BG1 character data at VRAM $0000
    lda #$00
    sta $210B               ; BG12NBA (BG1 uses low nibble)

    ; Load font into VRAM at $0000
    jsr load_font

    ; Set text palette (entries 0-3 for 2bpp)
    stz $2121               ; CGRAM address 0
    ; Color 0: Black (background)
    stz $2122
    stz $2122
    ; Color 1: White (text foreground)
    lda #$FF
    sta $2122
    lda #$7F
    sta $2122
    ; Color 2: Gray (unused)
    lda #$10
    sta $2122
    lda #$42
    sta $2122
    ; Color 3: Gray (unused)
    lda #$10
    sta $2122
    lda #$42
    sta $2122

    ; === SPRITE SETUP ===
    ; OBSEL: sprite size and base address
    ; Base 0 = VRAM $0000 for sprites (won't conflict with font at $0000-$0FFF)
    ; Actually, let's use a different base to avoid conflict
    lda #$03                ; Base 3 = VRAM word $6000, size=0 (8x8/16x16)
    sta $2101               ; OBSEL

    ; Set sprite palette 0, color 15 to yellow
    ; Sprite palettes start at CGRAM 128
    lda #$80 + 15           ; CGRAM address 128 + 15 = 143
    sta $2121
    lda #$FF                ; Yellow low byte
    sta $2122
    lda #$03                ; Yellow high byte
    sta $2122

    ; Enable BG1 and sprites on main screen
    lda #$11                ; Bit 0 = BG1, Bit 4 = OBJ
    sta $212C               ; TM = BG1 + OBJ enabled

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


; Load font data into VRAM at $0000
; Called with 8-bit A mode
load_font:
    php                     ; Save processor status
    rep #$20                ; 16-bit A
    .a16

    ; Set VRAM address to $0000
    lda #$0000
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
