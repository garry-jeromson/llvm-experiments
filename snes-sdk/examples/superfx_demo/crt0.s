; SNES SuperFX Demo - C Runtime Startup
; Sets up display for SuperFX demo UI

.p816
.smart

; Import main function from LLVM-compiled code
.import main

; Import font data from SDK
.import font_data, font_data_size

; GSU program is defined in RODATA section below

; Export entry points
.export reset, nmi_handler, irq_handler

; Export SuperFX control functions (called from C++)
.export _sfx_upload_and_run
.export _sfx_is_running
.export _sfx_copy_framebuffer

; Export SuperFX driver global state variables
.export _ZN4snes7superfx17g_sfx_initializedE
.export _ZN4snes7superfx13g_sfx_versionE

; SuperFX register addresses
SFR_LO   = $3030    ; Status/Flag Register low byte
SFR_HI   = $3031    ; Status/Flag Register high byte
BRAMR    = $3033    ; Backup RAM register
PBR      = $3034    ; Program Bank Register
ROMBR    = $3036    ; ROM Bank Register
CFGR     = $3037    ; Config Register
SCBR     = $3038    ; Screen Base Register
CLSR     = $3039    ; Clock Speed Register
SCMR     = $303A    ; Screen Mode Register
VCR      = $303B    ; Version Code Register
RAMBR    = $303C    ; RAM Bank Register
CBR_LO   = $303E    ; Cache Base Register low
CBR_HI   = $303F    ; Cache Base Register high

; GSU R15 (Program Counter) - write to start execution
GSU_R15_LO = $301E
GSU_R15_HI = $301F

; SuperFX RAM mirror in bank 0 (first 8KB of SuperFX RAM)
; This is much easier to access than the $70:0000 address
SFX_RAM_MIRROR = $6000

; SuperFX driver state (in BSS segment)
.segment "BSS"
_ZN4snes7superfx17g_sfx_initializedE: .res 1
_ZN4snes7superfx13g_sfx_versionE: .res 1

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
    stz $2118
    stz $2118
    inx
    cpx #$4000
    bne @clear_vram

    ; Clear CGRAM (palette)
    sep #$20
    .a8
    stz $2121
    ldx #$0200
@clear_cgram:
    stz $2122
    dex
    bne @clear_cgram

    ; Clear OAM
    stz $2102
    stz $2103
    ldx #$0080
@clear_oam:
    stz $2104
    lda #$F0
    sta $2104
    stz $2104
    stz $2104
    dex
    bne @clear_oam
    ldx #$0020
@clear_oam_high:
    stz $2104
    dex
    bne @clear_oam_high

    ; Set Mode 0
    lda #$00
    sta $2105

    ; BG1 tilemap at $1000
    lda #$10
    sta $2107

    ; BG1 tiles at $0000
    lda #$00
    sta $210B

    ; Load font
    jsr load_font

    ; Load UI tilemap
    jsr load_ui_tilemap

    ; Load palettes
    jsr load_ui_palette
    jsr load_sprite_palette

    ; Sprite setup
    lda #$03
    sta $2101

    ; Load sprites
    jsr load_ui_sprites

    ; Enable BG1 and sprites
    lda #$11
    sta $212C

    ; 16-bit mode for main
    rep #$30
    .a16
    .i16

    jsr main

forever:
    wai
    bra forever


; =============================================================================
; SuperFX Control Functions
; =============================================================================

; _sfx_upload_and_run - Upload GSU program to SuperFX RAM and execute it
; Called from C++ as: extern "C" void _sfx_upload_and_run();
_sfx_upload_and_run:
    php
    sep #$20                ; 8-bit A
    .a8
    rep #$10                ; 16-bit X/Y
    .i16

    ; DEBUG: Just return immediately to test if UI works
    ; Uncomment the code below once we verify the UI isn't crashing
    plp
    rts

    ; Wait for SuperFX to be idle first (with timeout)
    ldx #$FFFF              ; Timeout counter
@wait_sfx_idle:
    dex
    beq @timeout            ; Give up after timeout
    lda SFR_LO
    and #$20
    bne @wait_sfx_idle
@timeout:

    ; Configure SuperFX for 8bpp 128x128 mode
    lda #$00
    sta RAMBR               ; RAM bank 0 ($70:xxxx)
    lda #$00
    sta SCBR                ; Frame buffer at $0000
    lda #$1B                ; 8bpp mode ($03) + RAM enable ($08) + ROM enable ($10)
    sta SCMR

    ; Copy GSU program to SuperFX RAM via $6000 mirror
    ; The first 8KB of SuperFX RAM ($70:0000-$70:1FFF) is mirrored at $6000-$7FFF
    ldx #$0000
@copy_loop:
    lda gsu_fill_program,x
    sta SFX_RAM_MIRROR,x    ; Write to $6000+ mirror
    inx
    cpx #gsu_fill_program_size
    bcc @copy_loop

    ; Set program bank to $70 (SuperFX RAM)
    lda #$70
    sta PBR

    ; Set program counter (R15) to $0000 and START execution
    ; Writing to R15 automatically triggers SuperFX to start running
    lda #$00
    sta GSU_R15_LO
    sta GSU_R15_HI          ; This write starts execution!

    plp
    rts


; _sfx_is_running - Check if SuperFX is still running
; Returns: A = 1 if running, 0 if idle
_sfx_is_running:
    php
    sep #$20
    .a8
    lda SFR_LO
    and #$20                ; GO flag
    beq @not_running
    lda #$01
    bra @done
@not_running:
    lda #$00
@done:
    rep #$20
    .a16
    and #$00FF              ; Zero-extend to 16-bit
    plp
    rts


; _sfx_copy_framebuffer - Copy SuperFX frame buffer to VRAM using DMA
; Copies 128x128 8bpp = 16KB from $70:0000 to VRAM $0000
_sfx_copy_framebuffer:
    php
    sep #$20
    .a8

    ; Wait for SuperFX to finish
@wait_done:
    lda SFR_LO
    and #$20
    bne @wait_done

    ; Set up VRAM address
    lda #$80
    sta $2115               ; VRAM increment mode
    rep #$20
    .a16
    lda #$0000
    sta $2116               ; VRAM address = 0
    sep #$20
    .a8

    ; Set up DMA channel 0 to copy from SuperFX RAM to VRAM
    lda #$01                ; DMA mode: 2 bytes to $2118/$2119
    sta $4300
    lda #$18                ; Destination: $2118 (VRAM data)
    sta $4301

    ; Source address: $70:0000
    rep #$20
    .a16
    lda #$0000
    sta $4302               ; Source address low
    sep #$20
    .a8
    lda #$70
    sta $4304               ; Source bank

    ; Transfer size: 16384 bytes (128x128)
    rep #$20
    .a16
    lda #$4000
    sta $4305               ; Size
    sep #$20
    .a8

    ; Start DMA
    lda #$01
    sta $420B

    plp
    rts


load_font:
    php
    rep #$20
    .a16
    lda #$0000
    sta $2116
    sep #$20
    .a8
    lda #$80
    sta $2115
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

    ; Clear tilemap
    ldx #0
@clear:
    stz $2118
    inx
    cpx #1024
    bcc @clear

    ; Title "SUPERFX DEMO"
    lda #$1054
    sta $2116
    sep #$20
    .a8

    lda #('S'-32)
    sta $2118
    stz $2119
    lda #('U'-32)
    sta $2118
    stz $2119
    lda #('P'-32)
    sta $2118
    stz $2119
    lda #('E'-32)
    sta $2118
    stz $2119
    lda #('R'-32)
    sta $2118
    stz $2119
    lda #('F'-32)
    sta $2118
    stz $2119
    lda #('X'-32)
    sta $2118
    stz $2119
    lda #(' '-32)
    sta $2118
    stz $2119
    lda #('D'-32)
    sta $2118
    stz $2119
    lda #('E'-32)
    sta $2118
    stz $2119
    lda #('M'-32)
    sta $2118
    stz $2119
    lda #('O'-32)
    sta $2118
    stz $2119

    ; Status text
    rep #$20
    .a16
    lda #$10C2
    sta $2116
    sep #$20
    .a8

    ; "A:RUN B:STOP X:SPEED"
    lda #('A'-32)
    sta $2118
    stz $2119
    lda #(':'-32)
    sta $2118
    stz $2119
    lda #('R'-32)
    sta $2118
    stz $2119
    lda #('U'-32)
    sta $2118
    stz $2119
    lda #('N'-32)
    sta $2118
    stz $2119

    plp
    rts


load_ui_palette:
    php
    sep #$20
    .a8
    stz $2121

    ; Color 0: Dark green background
    lda #$00
    sta $2122
    lda #$18
    sta $2122

    ; Color 1: White
    lda #$FF
    sta $2122
    lda #$7F
    sta $2122

    ; Color 2: Gray
    lda #$94
    sta $2122
    lda #$52
    sta $2122

    ; Color 3: Yellow
    lda #$FF
    sta $2122
    lda #$03
    sta $2122

    plp
    rts


load_sprite_palette:
    php
    sep #$20
    .a8
    lda #128
    sta $2121

    ; Color 0: Transparent
    stz $2122
    stz $2122

    ; Color 1: Cyan (SuperFX indicator)
    lda #$FF
    sta $2122
    lda #$7F
    sta $2122

    ; Color 2: Orange
    lda #$1F
    sta $2122
    lda #$02
    sta $2122

    ; Color 3: White
    lda #$FF
    sta $2122
    lda #$7F
    sta $2122

    plp
    rts


load_ui_sprites:
    php
    rep #$20
    .a16
    lda #$6000
    sta $2116
    sep #$20
    .a8
    lda #$80
    sta $2115

    ; Tile 0: Circle indicator
    lda #$3C
    sta $2118
    sta $2119
    lda #$7E
    sta $2118
    sta $2119
    lda #$FF
    sta $2118
    sta $2119
    lda #$FF
    sta $2118
    sta $2119
    lda #$FF
    sta $2118
    sta $2119
    lda #$FF
    sta $2118
    sta $2119
    lda #$7E
    sta $2118
    sta $2119
    lda #$3C
    sta $2118
    sta $2119

    plp
    rts


nmi_handler:
    rti

irq_handler:
    rti


.segment "HEADER"
    ; SNES ROM Header at $FFB0-$FFDF
    ; Maker code (2 bytes)
    .byte "  "
    ; Game code (4 bytes)
    .byte "SFXD"
    ; Reserved (7 bytes)
    .byte $00,$00,$00,$00,$00,$00,$00
    ; Expansion RAM size
    .byte $00
    ; Special version
    .byte $00
    ; Cartridge sub-type
    .byte $00

    ; Title (21 bytes, space padded)
    .byte "SUPERFX DEMO         "

    ; Map mode: $20 = LoROM, 2.68MHz
    .byte $20
    ; Cartridge type: $00 = ROM only (DEBUG: changed from $13 SuperFX)
    ; $13 = Super FX (causes special memory mapping in emulator)
    .byte $00
    ; ROM size: $05 = 32KB (must match actual ROM size for correct memory mapping)
    .byte $05
    ; RAM size: $00 = none (DEBUG: changed from $05)
    .byte $00
    ; Region: $01 = USA
    .byte $01
    ; Developer ID
    .byte $00
    ; Version
    .byte $00
    ; Checksum complement (will be fixed)
    .byte $00,$00
    ; Checksum (will be fixed)
    .byte $00,$00


.segment "VECTORS"
    .word $0000
    .word $0000
    .word $0000
    .word $0000
    .word $0000
    .word nmi_handler
    .word $0000
    .word irq_handler

    .word $0000
    .word $0000
    .word $0000
    .word $0000
    .word $0000
    .word nmi_handler
    .word reset
    .word irq_handler


; =============================================================================
; GSU (SuperFX) Program Data
; =============================================================================
; This is the actual SuperFX machine code that runs on the GSU coprocessor.
; It fills the frame buffer with a solid color.

.segment "RODATA"

gsu_fill_program:
    ; GSU program to fill frame buffer with a solid color
    ;
    ; GSU Opcode Reference (from sneslab.net):
    ;   $00       = STOP
    ;   $01       = NOP
    ;   $1x       = TO Rx - set destination register
    ;   $3x       = STW (Rx) - store SREG word to RAM at Rx
    ;   $3C       = LOOP - dec R12, branch to R13 if not zero
    ;   $Ax bb    = IBT Rx, #imm8
    ;   $Bx       = FROM Rx - set source register
    ;   $Dx       = INC Rx
    ;   $Fx ww ww = IWT Rx, #imm16
    ;
    ; To do MOVE Rd, Rs: FROM Rs ($Bx), then TO Rd ($1x)
    ;
    ; Program flow:
    ; 1. R3 = 0 (destination pointer in SuperFX RAM)
    ; 2. R12 = loop count
    ; 3. R0 = fill value (repeated byte pattern)
    ; 4. Save PC to R13 (for LOOP)
    ; 5. Store R0 to (R3), increment R3
    ; 6. LOOP back to step 5
    ; 7. STOP

    ; IWT R3, #$0000 - destination pointer
    .byte $F3
    .word $0000

    ; IWT R12, #$1000 - loop counter (4096 iterations * 2 bytes = 8KB)
    .byte $FC
    .word $1000

    ; IWT R0, #$1F1F - fill value (color 31 in both bytes)
    .byte $F0
    .word $1F1F

    ; FROM R0 - set SREG to R0 (for STW to use)
    .byte $B0

    ; MOVE R13, R15 - save loop address to R13
    ; FROM R15 = $BF, TO R13 = $1D
    .byte $BF               ; FROM R15 (program counter)
    .byte $1D               ; TO R13 (loop target register)

    ; --- Loop body starts here ---
    ; The TO instruction above already executed, so R13 now points here

    ; STW (R3) - store SREG (=R0) to RAM at address R3
    .byte $33

    ; INC R3 - increment pointer (STW is word, so +2)
    .byte $D3
    .byte $D3

    ; LOOP - dec R12, branch to R13 if R12 != 0
    .byte $3C

    ; STOP
    .byte $00

    ; NOP padding
    .byte $01

gsu_fill_program_end:

; Size constant for the copy loop
gsu_fill_program_size = gsu_fill_program_end - gsu_fill_program
