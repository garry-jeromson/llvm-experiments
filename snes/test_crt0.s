; SNES Test ROM - Self-contained
; Just sets screen to bright green, no C code needed

.p816
.smart

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

    ; PPU initialization
    sep #$20                ; 8-bit A
    .a8

    lda #$80
    sta $2100               ; Force blank (screen off)

    stz $4200               ; Disable NMI/IRQ
    stz $420B               ; Disable DMA
    stz $420C               ; Disable HDMA

    ; Set CGRAM address to 0 (backdrop color)
    stz $2121

    ; Write bright green color: BGR555 = 0x03E0
    ; Green = 31 max, so: (31 << 5) = 0x03E0
    ; Low byte first
    lda #$E0
    sta $2122
    ; High byte
    lda #$03
    sta $2122

    ; Turn screen on
    lda #$0F                ; Full brightness, no force blank
    sta $2100

    ; Infinite loop
forever:
    wai
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
    .byte "TEST"            ; $FFB2-$FFB5: Game code (4 bytes)
    .byte $00,$00,$00,$00,$00,$00,$00  ; $FFB6-$FFBC: Reserved (7 bytes)
    .byte $00               ; $FFBD: Expansion RAM size
    .byte $00               ; $FFBE: Special version
    .byte $00               ; $FFBF: Cartridge sub-type

    ; $FFC0-$FFD4: Game title (21 bytes, padded with spaces)
    .byte "SNES TEST ROM        "

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
