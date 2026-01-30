; SNES Sprite Graphics Data
; 8x8 smiley face sprite in 4bpp format (SNES sprites are always 4bpp)

.p816
.smart

.export sprite_data, sprite_data_size

.segment "RODATA"

; 8x8 Smiley Face - 4bpp format (32 bytes)
;
; SNES 4bpp tile format:
;   First 16 bytes: bitplanes 0 and 1 interleaved (row by row)
;   Next 16 bytes:  bitplanes 2 and 3 interleaved (row by row)
;
; Design (8x8) - # = filled, . = empty:
;   Row 0:   . . # # # # . .   = $3C
;   Row 1:   . # # # # # # .   = $7E
;   Row 2:   # # . # # . # #   = $DB (eyes = holes)
;   Row 3:   # # # # # # # #   = $FF
;   Row 4:   # # . # # . # #   = $DB (mouth corners)
;   Row 5:   # # # . . # # #   = $E7 (mouth)
;   Row 6:   . # # # # # # .   = $7E
;   Row 7:   . . # # # # . .   = $3C
;
; Using color 15 (all 4 bitplanes set) for solid yellow

sprite_data:
    ; Bitplanes 0 and 1 (first 16 bytes)
    .byte $3C, $3C    ; Row 0: bp0, bp1
    .byte $7E, $7E    ; Row 1
    .byte $DB, $DB    ; Row 2 (eyes)
    .byte $FF, $FF    ; Row 3
    .byte $DB, $DB    ; Row 4 (mouth corners)
    .byte $E7, $E7    ; Row 5 (mouth)
    .byte $7E, $7E    ; Row 6
    .byte $3C, $3C    ; Row 7

    ; Bitplanes 2 and 3 (next 16 bytes)
    .byte $3C, $3C    ; Row 0: bp2, bp3
    .byte $7E, $7E    ; Row 1
    .byte $DB, $DB    ; Row 2
    .byte $FF, $FF    ; Row 3
    .byte $DB, $DB    ; Row 4
    .byte $E7, $E7    ; Row 5
    .byte $7E, $7E    ; Row 6
    .byte $3C, $3C    ; Row 7

sprite_data_size = * - sprite_data
