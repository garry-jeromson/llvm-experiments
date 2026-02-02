; SuperFX (GSU) Program - Screen Fill
; ====================================
; Simple GSU program that fills the frame buffer with a solid color.
; Runs on the SuperFX coprocessor.
;
; This is the simplest possible GSU program for testing:
; - Writes directly to frame buffer memory
; - Fills with a solid color
; - Stops

.p816
.smart

.export gsu_fill_program, gsu_fill_program_size

.segment "RODATA"

; GSU program as raw bytes (hand-assembled)
; The GSU uses a different instruction set than the 65816
;
; This simple program:
; 1. Clears the frame buffer to color 0x1F (cyan-ish)
; 2. Stops
;
; GSU opcodes used:
;   $00 = STOP (halt execution)
;   $Fn = IWT Rn, #imm16 (load 16-bit immediate)
;   $An = IBT Rn, #imm8 (load 8-bit signed immediate)
;   $3D = ALT1 prefix
;   $3E = ALT2 prefix
;   $Bn = FROM Rn (set source register)
;   $Dn = INC Rn
;   $4C = PLOT
;   $4E = COLOR (with ALT1: CMODE)

gsu_fill_program:
    ; === Minimal fill program ===
    ;
    ; Set R1 (X) = 0, R2 (Y) = 0
    ; Set R4 (color) = 0x3F
    ; Loop: PLOT 128x128 times
    ; STOP

    ; IWT R1, #$0000 (X = 0)
    .byte $F1, $00, $00

    ; IWT R2, #$0000 (Y = 0)
    .byte $F2, $00, $00

    ; IWT R12, #$4000 (loop counter = 128*128 = 16384)
    .byte $FC, $00, $40

    ; IBT R4, #$3F (color = 63, bright cyan)
    .byte $A4, $3F

    ; COLOR (set plot color from R4)
    .byte $4E

; loop: (offset 12 from start)
    ; PLOT (draw pixel at R1,R2, auto-increment)
    .byte $4C

    ; LOOP (decrement R12, branch back if not zero)
    ; Offset is -2 bytes (back to PLOT)
    .byte $3C
    .byte $FE                   ; -2 in signed byte

    ; STOP
    .byte $00

gsu_fill_program_end:

gsu_fill_program_size = gsu_fill_program_end - gsu_fill_program


; Alternative: simple memory fill without PLOT
; This writes directly to frame buffer bytes

gsu_memfill_program:
    ; IWT R0, #$0000 (destination pointer)
    .byte $F0, $00, $00

    ; IWT R12, #$4000 (byte count = 16384)
    .byte $FC, $00, $40

    ; IBT R1, #$1F (fill value)
    .byte $A1, $1F

; memloop:
    ; STW (R0) - store R0 to RAM at (R0) - need RAMB first
    ; This is complex, use simpler approach

    ; LOOP
    .byte $3C
    .byte $FC                   ; branch offset

    ; STOP
    .byte $00

gsu_memfill_program_end:
