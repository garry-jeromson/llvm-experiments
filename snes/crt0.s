; SNES C Runtime Startup
; Initializes hardware and calls main()

.p816
.smart

; Import main function from LLVM-compiled code
.import main

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

    ; Clear OAM
    stz $2102               ; OAM address low
    stz $2103               ; OAM address high
    ldx #$0220              ; 544 bytes
@clear_oam:
    stz $2104
    dex
    bne @clear_oam

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
    .byte "LLVM W65816 DEMO     "

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
