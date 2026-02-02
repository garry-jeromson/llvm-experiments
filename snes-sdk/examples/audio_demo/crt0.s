; SNES Audio Demo - C Runtime Startup
; Sets up display and sprite graphics for audio demo UI

.p816
.smart

; Import main function from LLVM-compiled code
.import main

; Import font data from SDK
.import font_data, font_data_size

; Export entry points
.export reset, nmi_handler, irq_handler

; Export audio driver global state variables
.export _ZN4snes5audio19g_audio_initializedE
.export _ZN4snes5audio21g_audio_master_volumeE
.export _ZN4snes5audio23g_audio_command_counterE

; Audio driver state (in BSS segment)
.segment "BSS"
_ZN4snes5audio19g_audio_initializedE:   .res 1
_ZN4snes5audio21g_audio_master_volumeE: .res 1
_ZN4snes5audio23g_audio_command_counterE: .res 1

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
    lda #$10
    sta $2107               ; BG1SC

    ; BG1 character data at VRAM $0000
    lda #$00
    sta $210B               ; BG12NBA

    ; Load font tiles to VRAM $0000 (for background text)
    jsr load_font

    ; Load UI tilemap
    jsr load_ui_tilemap

    ; Load palettes
    jsr load_ui_palette
    jsr load_sprite_palette

    ; === SPRITE SETUP ===
    ; OBSEL: sprite size and base address
    lda #$03                ; Base 3 = VRAM word $6000, size=0 (8x8/16x16)
    sta $2101               ; OBSEL

    ; Load sprite graphics
    jsr load_ui_sprites

    ; Enable BG1 and sprites on main screen
    lda #$11                ; Bit 0 = BG1, Bit 4 = OBJ
    sta $212C               ; TM

    ; Set up registers for 16-bit mode
    rep #$30                ; 16-bit A/X/Y
    .a16
    .i16

    ; Call main function
    jsr main

    ; Main returned - enter infinite loop
forever:
    wai
    bra forever


; Load font data into VRAM at $0000
load_font:
    php
    rep #$20
    .a16

    lda #$0000
    sta $2116               ; VRAM address

    sep #$20
    .a8
    lda #$80
    sta $2115               ; VMAIN

    rep #$20
    .a16

    ldx #0
@font_loop:
    lda font_data,x
    sta $2118
    inx
    inx
    cpx #font_data_size
    bcc @font_loop

    plp
    rts


; Load UI tilemap into VRAM at $1000
load_ui_tilemap:
    php
    rep #$20
    .a16

    lda #$1000
    sta $2116

    sep #$20
    .a8
    lda #$80
    sta $2115

    rep #$20
    .a16

    ; Clear tilemap first
    ldx #0
@clear:
    stz $2118
    inx
    cpx #1024
    bcc @clear

    ; Write title "AUDIO DEMO" at row 2, col 10
    lda #$1054              ; $1000 + (2*32 + 20)/2 * 2
    sta $2116
    sep #$20
    .a8

    ; "AUDIO DEMO" with palette 0
    lda #('A'-32)
    sta $2118
    lda #$00
    sta $2119
    lda #('U'-32)
    sta $2118
    lda #$00
    sta $2119
    lda #('D'-32)
    sta $2118
    lda #$00
    sta $2119
    lda #('I'-32)
    sta $2118
    lda #$00
    sta $2119
    lda #('O'-32)
    sta $2118
    lda #$00
    sta $2119
    lda #(' '-32)
    sta $2118
    lda #$00
    sta $2119
    lda #('D'-32)
    sta $2118
    lda #$00
    sta $2119
    lda #('E'-32)
    sta $2118
    lda #$00
    sta $2119
    lda #('M'-32)
    sta $2118
    lda #$00
    sta $2119
    lda #('O'-32)
    sta $2118
    lda #$00
    sta $2119

    ; "VOLUME:" at row 6, col 2
    rep #$20
    .a16
    lda #$10C2              ; $1000 + (6*32 + 2)
    sta $2116
    sep #$20
    .a8

    lda #('V'-32)
    sta $2118
    lda #$00
    sta $2119
    lda #('O'-32)
    sta $2118
    lda #$00
    sta $2119
    lda #('L'-32)
    sta $2118
    lda #$00
    sta $2119
    lda #('U'-32)
    sta $2118
    lda #$00
    sta $2119
    lda #('M'-32)
    sta $2118
    lda #$00
    sta $2119
    lda #('E'-32)
    sta $2118
    lda #$00
    sta $2119
    lda #(':'-32)
    sta $2118
    lda #$00
    sta $2119

    ; "TRACK:" at row 10, col 2
    rep #$20
    .a16
    lda #$1142              ; $1000 + (10*32 + 2)
    sta $2116
    sep #$20
    .a8

    lda #('T'-32)
    sta $2118
    lda #$00
    sta $2119
    lda #('R'-32)
    sta $2118
    lda #$00
    sta $2119
    lda #('A'-32)
    sta $2118
    lda #$00
    sta $2119
    lda #('C'-32)
    sta $2118
    lda #$00
    sta $2119
    lda #('K'-32)
    sta $2118
    lda #$00
    sta $2119
    lda #(':'-32)
    sta $2118
    lda #$00
    sta $2119

    ; Controls help text at bottom
    rep #$20
    .a16
    lda #$1302              ; $1000 + (24*32 + 2)
    sta $2116
    sep #$20
    .a8

    ; "A/B/X/Y:SFX  START:PLAY"
    lda #('A'-32)
    sta $2118
    stz $2119
    lda #('/'-32)
    sta $2118
    stz $2119
    lda #('B'-32)
    sta $2118
    stz $2119
    lda #('/'-32)
    sta $2118
    stz $2119
    lda #('X'-32)
    sta $2118
    stz $2119
    lda #('/'-32)
    sta $2118
    stz $2119
    lda #('Y'-32)
    sta $2118
    stz $2119
    lda #(':'-32)
    sta $2118
    stz $2119
    lda #('S'-32)
    sta $2118
    stz $2119
    lda #('F'-32)
    sta $2118
    stz $2119
    lda #('X'-32)
    sta $2118
    stz $2119

    plp
    rts


; Load UI palette
load_ui_palette:
    php
    sep #$20
    .a8

    stz $2121               ; CGRAM address = 0

    ; Color 0: Dark blue background
    lda #$00
    sta $2122
    lda #$28                ; Dark blue
    sta $2122

    ; Color 1: White (text)
    lda #$FF
    sta $2122
    lda #$7F
    sta $2122

    ; Color 2: Gray
    lda #$94
    sta $2122
    lda #$52
    sta $2122

    ; Color 3: Light blue
    lda #$FF
    sta $2122
    lda #$7B
    sta $2122

    plp
    rts


; Load sprite palette (palette 3, CGRAM 48-51 for 2bpp Mode 0)
load_sprite_palette:
    php
    sep #$20
    .a8

    lda #128                ; Sprite palettes start at CGRAM 128
    sta $2121

    ; Color 0: Transparent
    lda #$00
    sta $2122
    sta $2122

    ; Color 1: Green (volume bar)
    lda #$E0
    sta $2122
    lda #$03
    sta $2122

    ; Color 2: Yellow
    lda #$FF
    sta $2122
    lda #$03
    sta $2122

    ; Color 3: Red
    lda #$1F
    sta $2122
    lda #$00
    sta $2122

    plp
    rts


; Load UI sprite graphics
load_ui_sprites:
    php
    rep #$20
    .a16

    lda #$6000              ; Sprite tiles at VRAM $6000
    sta $2116

    sep #$20
    .a8
    lda #$80
    sta $2115

    ; Tile 0: Volume bar (solid block)
    ; 8x8 tile, 2bpp = 16 bytes
    lda #$FF
    ldx #8
@tile0_loop:
    sta $2118               ; Bitplane 0
    sta $2118               ; Bitplane 1
    dex
    bne @tile0_loop

    ; Tile 1: Play indicator (triangle)
    ; Row pattern for play triangle - 8 rows, 2 bytes per row
    lda #$80
    sta $2118
    sta $2119
    lda #$C0
    sta $2118
    sta $2119
    lda #$E0
    sta $2118
    sta $2119
    lda #$F0
    sta $2118
    sta $2119
    lda #$E0
    sta $2118
    sta $2119
    lda #$C0
    sta $2118
    sta $2119
    lda #$80
    sta $2118
    sta $2119
    lda #$00
    sta $2118
    sta $2119

    plp
    rts


; NMI Handler (VBlank)
nmi_handler:
    rti

; IRQ Handler
irq_handler:
    rti


.segment "HEADER"
    ; SNES ROM Header
    .byte "  "              ; Maker code
    .byte "AUDI"            ; Game code
    .byte $00,$00,$00,$00,$00,$00,$00
    .byte $00               ; Expansion RAM size
    .byte $00               ; Special version
    .byte $00               ; Cartridge sub-type

    .byte "AUDIO DEMO"      ; 10 bytes
    .byte "           "     ; 11 spaces = 21 total

    .byte $20               ; Map mode (LoROM)
    .byte $00               ; Cartridge type (ROM only)
    .byte $05               ; ROM size (32KB)
    .byte $00               ; RAM size (0)
    .byte $01               ; Destination code
    .byte $00               ; Fixed value
    .byte $00               ; Version number
    .byte $00,$00           ; Checksum complement
    .byte $00,$00           ; Checksum


.segment "VECTORS"
    ; Native mode vectors
    .word $0000
    .word $0000
    .word $0000
    .word $0000
    .word $0000
    .word nmi_handler
    .word $0000
    .word irq_handler

    ; Emulation mode vectors
    .word $0000
    .word $0000
    .word $0000
    .word $0000
    .word $0000
    .word nmi_handler
    .word reset
    .word irq_handler
