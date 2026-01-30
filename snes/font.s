; 8x8 2bpp Font for SNES
; Format: SNES 2bpp planar (interleaved)
; Each character: 16 bytes (row0-bp0, row0-bp1, row1-bp0, row1-bp1, ...)
; Characters: ASCII 32-95 (space through underscore, including 0-9, A-Z)

.p816
.smart

.segment "RODATA"

.export font_data, font_data_size

font_data:

; Character 32: Space
.byte $00, $00  ; row 0
.byte $00, $00  ; row 1
.byte $00, $00  ; row 2
.byte $00, $00  ; row 3
.byte $00, $00  ; row 4
.byte $00, $00  ; row 5
.byte $00, $00  ; row 6
.byte $00, $00  ; row 7

; Character 33: !
.byte $18, $00  ; row 0
.byte $18, $00  ; row 1
.byte $18, $00  ; row 2
.byte $18, $00  ; row 3
.byte $18, $00  ; row 4
.byte $00, $00  ; row 5
.byte $18, $00  ; row 6
.byte $00, $00  ; row 7

; Character 34: "
.byte $6C, $00  ; row 0
.byte $6C, $00  ; row 1
.byte $6C, $00  ; row 2
.byte $00, $00  ; row 3
.byte $00, $00  ; row 4
.byte $00, $00  ; row 5
.byte $00, $00  ; row 6
.byte $00, $00  ; row 7

; Character 35: #
.byte $6C, $00  ; row 0
.byte $6C, $00  ; row 1
.byte $FE, $00  ; row 2
.byte $6C, $00  ; row 3
.byte $FE, $00  ; row 4
.byte $6C, $00  ; row 5
.byte $6C, $00  ; row 6
.byte $00, $00  ; row 7

; Character 36: $
.byte $18, $00  ; row 0
.byte $3E, $00  ; row 1
.byte $60, $00  ; row 2
.byte $3C, $00  ; row 3
.byte $06, $00  ; row 4
.byte $7C, $00  ; row 5
.byte $18, $00  ; row 6
.byte $00, $00  ; row 7

; Character 37: %
.byte $00, $00  ; row 0
.byte $C6, $00  ; row 1
.byte $CC, $00  ; row 2
.byte $18, $00  ; row 3
.byte $30, $00  ; row 4
.byte $66, $00  ; row 5
.byte $C6, $00  ; row 6
.byte $00, $00  ; row 7

; Character 38: &
.byte $38, $00  ; row 0
.byte $6C, $00  ; row 1
.byte $38, $00  ; row 2
.byte $76, $00  ; row 3
.byte $DC, $00  ; row 4
.byte $CC, $00  ; row 5
.byte $76, $00  ; row 6
.byte $00, $00  ; row 7

; Character 39: '
.byte $18, $00  ; row 0
.byte $18, $00  ; row 1
.byte $30, $00  ; row 2
.byte $00, $00  ; row 3
.byte $00, $00  ; row 4
.byte $00, $00  ; row 5
.byte $00, $00  ; row 6
.byte $00, $00  ; row 7

; Character 40: (
.byte $0C, $00  ; row 0
.byte $18, $00  ; row 1
.byte $30, $00  ; row 2
.byte $30, $00  ; row 3
.byte $30, $00  ; row 4
.byte $18, $00  ; row 5
.byte $0C, $00  ; row 6
.byte $00, $00  ; row 7

; Character 41: )
.byte $30, $00  ; row 0
.byte $18, $00  ; row 1
.byte $0C, $00  ; row 2
.byte $0C, $00  ; row 3
.byte $0C, $00  ; row 4
.byte $18, $00  ; row 5
.byte $30, $00  ; row 6
.byte $00, $00  ; row 7

; Character 42: *
.byte $00, $00  ; row 0
.byte $66, $00  ; row 1
.byte $3C, $00  ; row 2
.byte $FF, $00  ; row 3
.byte $3C, $00  ; row 4
.byte $66, $00  ; row 5
.byte $00, $00  ; row 6
.byte $00, $00  ; row 7

; Character 43: +
.byte $00, $00  ; row 0
.byte $18, $00  ; row 1
.byte $18, $00  ; row 2
.byte $7E, $00  ; row 3
.byte $18, $00  ; row 4
.byte $18, $00  ; row 5
.byte $00, $00  ; row 6
.byte $00, $00  ; row 7

; Character 44: ,
.byte $00, $00  ; row 0
.byte $00, $00  ; row 1
.byte $00, $00  ; row 2
.byte $00, $00  ; row 3
.byte $00, $00  ; row 4
.byte $18, $00  ; row 5
.byte $18, $00  ; row 6
.byte $30, $00  ; row 7

; Character 45: -
.byte $00, $00  ; row 0
.byte $00, $00  ; row 1
.byte $00, $00  ; row 2
.byte $7E, $00  ; row 3
.byte $00, $00  ; row 4
.byte $00, $00  ; row 5
.byte $00, $00  ; row 6
.byte $00, $00  ; row 7

; Character 46: .
.byte $00, $00  ; row 0
.byte $00, $00  ; row 1
.byte $00, $00  ; row 2
.byte $00, $00  ; row 3
.byte $00, $00  ; row 4
.byte $18, $00  ; row 5
.byte $18, $00  ; row 6
.byte $00, $00  ; row 7

; Character 47: /
.byte $06, $00  ; row 0
.byte $0C, $00  ; row 1
.byte $18, $00  ; row 2
.byte $30, $00  ; row 3
.byte $60, $00  ; row 4
.byte $C0, $00  ; row 5
.byte $80, $00  ; row 6
.byte $00, $00  ; row 7

; Character 48: 0
.byte $7C, $00  ; row 0
.byte $C6, $00  ; row 1
.byte $CE, $00  ; row 2
.byte $D6, $00  ; row 3
.byte $E6, $00  ; row 4
.byte $C6, $00  ; row 5
.byte $7C, $00  ; row 6
.byte $00, $00  ; row 7

; Character 49: 1
.byte $18, $00  ; row 0
.byte $38, $00  ; row 1
.byte $18, $00  ; row 2
.byte $18, $00  ; row 3
.byte $18, $00  ; row 4
.byte $18, $00  ; row 5
.byte $7E, $00  ; row 6
.byte $00, $00  ; row 7

; Character 50: 2
.byte $7C, $00  ; row 0
.byte $C6, $00  ; row 1
.byte $06, $00  ; row 2
.byte $1C, $00  ; row 3
.byte $30, $00  ; row 4
.byte $60, $00  ; row 5
.byte $FE, $00  ; row 6
.byte $00, $00  ; row 7

; Character 51: 3
.byte $7C, $00  ; row 0
.byte $C6, $00  ; row 1
.byte $06, $00  ; row 2
.byte $3C, $00  ; row 3
.byte $06, $00  ; row 4
.byte $C6, $00  ; row 5
.byte $7C, $00  ; row 6
.byte $00, $00  ; row 7

; Character 52: 4
.byte $1C, $00  ; row 0
.byte $3C, $00  ; row 1
.byte $6C, $00  ; row 2
.byte $CC, $00  ; row 3
.byte $FE, $00  ; row 4
.byte $0C, $00  ; row 5
.byte $0C, $00  ; row 6
.byte $00, $00  ; row 7

; Character 53: 5
.byte $FE, $00  ; row 0
.byte $C0, $00  ; row 1
.byte $FC, $00  ; row 2
.byte $06, $00  ; row 3
.byte $06, $00  ; row 4
.byte $C6, $00  ; row 5
.byte $7C, $00  ; row 6
.byte $00, $00  ; row 7

; Character 54: 6
.byte $38, $00  ; row 0
.byte $60, $00  ; row 1
.byte $C0, $00  ; row 2
.byte $FC, $00  ; row 3
.byte $C6, $00  ; row 4
.byte $C6, $00  ; row 5
.byte $7C, $00  ; row 6
.byte $00, $00  ; row 7

; Character 55: 7
.byte $FE, $00  ; row 0
.byte $C6, $00  ; row 1
.byte $0C, $00  ; row 2
.byte $18, $00  ; row 3
.byte $30, $00  ; row 4
.byte $30, $00  ; row 5
.byte $30, $00  ; row 6
.byte $00, $00  ; row 7

; Character 56: 8
.byte $7C, $00  ; row 0
.byte $C6, $00  ; row 1
.byte $C6, $00  ; row 2
.byte $7C, $00  ; row 3
.byte $C6, $00  ; row 4
.byte $C6, $00  ; row 5
.byte $7C, $00  ; row 6
.byte $00, $00  ; row 7

; Character 57: 9
.byte $7C, $00  ; row 0
.byte $C6, $00  ; row 1
.byte $C6, $00  ; row 2
.byte $7E, $00  ; row 3
.byte $06, $00  ; row 4
.byte $0C, $00  ; row 5
.byte $78, $00  ; row 6
.byte $00, $00  ; row 7

; Character 58: :
.byte $00, $00  ; row 0
.byte $18, $00  ; row 1
.byte $18, $00  ; row 2
.byte $00, $00  ; row 3
.byte $00, $00  ; row 4
.byte $18, $00  ; row 5
.byte $18, $00  ; row 6
.byte $00, $00  ; row 7

; Character 59: ;
.byte $00, $00  ; row 0
.byte $18, $00  ; row 1
.byte $18, $00  ; row 2
.byte $00, $00  ; row 3
.byte $00, $00  ; row 4
.byte $18, $00  ; row 5
.byte $18, $00  ; row 6
.byte $30, $00  ; row 7

; Character 60: <
.byte $0C, $00  ; row 0
.byte $18, $00  ; row 1
.byte $30, $00  ; row 2
.byte $60, $00  ; row 3
.byte $30, $00  ; row 4
.byte $18, $00  ; row 5
.byte $0C, $00  ; row 6
.byte $00, $00  ; row 7

; Character 61: =
.byte $00, $00  ; row 0
.byte $00, $00  ; row 1
.byte $7E, $00  ; row 2
.byte $00, $00  ; row 3
.byte $7E, $00  ; row 4
.byte $00, $00  ; row 5
.byte $00, $00  ; row 6
.byte $00, $00  ; row 7

; Character 62: >
.byte $30, $00  ; row 0
.byte $18, $00  ; row 1
.byte $0C, $00  ; row 2
.byte $06, $00  ; row 3
.byte $0C, $00  ; row 4
.byte $18, $00  ; row 5
.byte $30, $00  ; row 6
.byte $00, $00  ; row 7

; Character 63: ?
.byte $7C, $00  ; row 0
.byte $C6, $00  ; row 1
.byte $0C, $00  ; row 2
.byte $18, $00  ; row 3
.byte $18, $00  ; row 4
.byte $00, $00  ; row 5
.byte $18, $00  ; row 6
.byte $00, $00  ; row 7

; Character 64: @
.byte $7C, $00  ; row 0
.byte $C6, $00  ; row 1
.byte $DE, $00  ; row 2
.byte $DE, $00  ; row 3
.byte $DC, $00  ; row 4
.byte $C0, $00  ; row 5
.byte $7C, $00  ; row 6
.byte $00, $00  ; row 7

; Character 65: A
.byte $38, $00  ; row 0
.byte $6C, $00  ; row 1
.byte $C6, $00  ; row 2
.byte $C6, $00  ; row 3
.byte $FE, $00  ; row 4
.byte $C6, $00  ; row 5
.byte $C6, $00  ; row 6
.byte $00, $00  ; row 7

; Character 66: B
.byte $FC, $00  ; row 0
.byte $C6, $00  ; row 1
.byte $C6, $00  ; row 2
.byte $FC, $00  ; row 3
.byte $C6, $00  ; row 4
.byte $C6, $00  ; row 5
.byte $FC, $00  ; row 6
.byte $00, $00  ; row 7

; Character 67: C
.byte $7C, $00  ; row 0
.byte $C6, $00  ; row 1
.byte $C0, $00  ; row 2
.byte $C0, $00  ; row 3
.byte $C0, $00  ; row 4
.byte $C6, $00  ; row 5
.byte $7C, $00  ; row 6
.byte $00, $00  ; row 7

; Character 68: D
.byte $F8, $00  ; row 0
.byte $CC, $00  ; row 1
.byte $C6, $00  ; row 2
.byte $C6, $00  ; row 3
.byte $C6, $00  ; row 4
.byte $CC, $00  ; row 5
.byte $F8, $00  ; row 6
.byte $00, $00  ; row 7

; Character 69: E
.byte $FE, $00  ; row 0
.byte $C0, $00  ; row 1
.byte $C0, $00  ; row 2
.byte $F8, $00  ; row 3
.byte $C0, $00  ; row 4
.byte $C0, $00  ; row 5
.byte $FE, $00  ; row 6
.byte $00, $00  ; row 7

; Character 70: F
.byte $FE, $00  ; row 0
.byte $C0, $00  ; row 1
.byte $C0, $00  ; row 2
.byte $F8, $00  ; row 3
.byte $C0, $00  ; row 4
.byte $C0, $00  ; row 5
.byte $C0, $00  ; row 6
.byte $00, $00  ; row 7

; Character 71: G
.byte $7C, $00  ; row 0
.byte $C6, $00  ; row 1
.byte $C0, $00  ; row 2
.byte $CE, $00  ; row 3
.byte $C6, $00  ; row 4
.byte $C6, $00  ; row 5
.byte $7E, $00  ; row 6
.byte $00, $00  ; row 7

; Character 72: H
.byte $C6, $00  ; row 0
.byte $C6, $00  ; row 1
.byte $C6, $00  ; row 2
.byte $FE, $00  ; row 3
.byte $C6, $00  ; row 4
.byte $C6, $00  ; row 5
.byte $C6, $00  ; row 6
.byte $00, $00  ; row 7

; Character 73: I
.byte $7E, $00  ; row 0
.byte $18, $00  ; row 1
.byte $18, $00  ; row 2
.byte $18, $00  ; row 3
.byte $18, $00  ; row 4
.byte $18, $00  ; row 5
.byte $7E, $00  ; row 6
.byte $00, $00  ; row 7

; Character 74: J
.byte $1E, $00  ; row 0
.byte $06, $00  ; row 1
.byte $06, $00  ; row 2
.byte $06, $00  ; row 3
.byte $C6, $00  ; row 4
.byte $C6, $00  ; row 5
.byte $7C, $00  ; row 6
.byte $00, $00  ; row 7

; Character 75: K
.byte $C6, $00  ; row 0
.byte $CC, $00  ; row 1
.byte $D8, $00  ; row 2
.byte $F0, $00  ; row 3
.byte $D8, $00  ; row 4
.byte $CC, $00  ; row 5
.byte $C6, $00  ; row 6
.byte $00, $00  ; row 7

; Character 76: L
.byte $C0, $00  ; row 0
.byte $C0, $00  ; row 1
.byte $C0, $00  ; row 2
.byte $C0, $00  ; row 3
.byte $C0, $00  ; row 4
.byte $C0, $00  ; row 5
.byte $FE, $00  ; row 6
.byte $00, $00  ; row 7

; Character 77: M
.byte $C6, $00  ; row 0
.byte $EE, $00  ; row 1
.byte $FE, $00  ; row 2
.byte $D6, $00  ; row 3
.byte $C6, $00  ; row 4
.byte $C6, $00  ; row 5
.byte $C6, $00  ; row 6
.byte $00, $00  ; row 7

; Character 78: N
.byte $C6, $00  ; row 0
.byte $E6, $00  ; row 1
.byte $F6, $00  ; row 2
.byte $DE, $00  ; row 3
.byte $CE, $00  ; row 4
.byte $C6, $00  ; row 5
.byte $C6, $00  ; row 6
.byte $00, $00  ; row 7

; Character 79: O
.byte $7C, $00  ; row 0
.byte $C6, $00  ; row 1
.byte $C6, $00  ; row 2
.byte $C6, $00  ; row 3
.byte $C6, $00  ; row 4
.byte $C6, $00  ; row 5
.byte $7C, $00  ; row 6
.byte $00, $00  ; row 7

; Character 80: P
.byte $FC, $00  ; row 0
.byte $C6, $00  ; row 1
.byte $C6, $00  ; row 2
.byte $FC, $00  ; row 3
.byte $C0, $00  ; row 4
.byte $C0, $00  ; row 5
.byte $C0, $00  ; row 6
.byte $00, $00  ; row 7

; Character 81: Q
.byte $7C, $00  ; row 0
.byte $C6, $00  ; row 1
.byte $C6, $00  ; row 2
.byte $C6, $00  ; row 3
.byte $D6, $00  ; row 4
.byte $CC, $00  ; row 5
.byte $76, $00  ; row 6
.byte $00, $00  ; row 7

; Character 82: R
.byte $FC, $00  ; row 0
.byte $C6, $00  ; row 1
.byte $C6, $00  ; row 2
.byte $FC, $00  ; row 3
.byte $D8, $00  ; row 4
.byte $CC, $00  ; row 5
.byte $C6, $00  ; row 6
.byte $00, $00  ; row 7

; Character 83: S
.byte $7C, $00  ; row 0
.byte $C6, $00  ; row 1
.byte $60, $00  ; row 2
.byte $38, $00  ; row 3
.byte $0C, $00  ; row 4
.byte $C6, $00  ; row 5
.byte $7C, $00  ; row 6
.byte $00, $00  ; row 7

; Character 84: T
.byte $7E, $00  ; row 0
.byte $18, $00  ; row 1
.byte $18, $00  ; row 2
.byte $18, $00  ; row 3
.byte $18, $00  ; row 4
.byte $18, $00  ; row 5
.byte $18, $00  ; row 6
.byte $00, $00  ; row 7

; Character 85: U
.byte $C6, $00  ; row 0
.byte $C6, $00  ; row 1
.byte $C6, $00  ; row 2
.byte $C6, $00  ; row 3
.byte $C6, $00  ; row 4
.byte $C6, $00  ; row 5
.byte $7C, $00  ; row 6
.byte $00, $00  ; row 7

; Character 86: V
.byte $C6, $00  ; row 0
.byte $C6, $00  ; row 1
.byte $C6, $00  ; row 2
.byte $C6, $00  ; row 3
.byte $6C, $00  ; row 4
.byte $38, $00  ; row 5
.byte $10, $00  ; row 6
.byte $00, $00  ; row 7

; Character 87: W
.byte $C6, $00  ; row 0
.byte $C6, $00  ; row 1
.byte $C6, $00  ; row 2
.byte $D6, $00  ; row 3
.byte $FE, $00  ; row 4
.byte $EE, $00  ; row 5
.byte $C6, $00  ; row 6
.byte $00, $00  ; row 7

; Character 88: X
.byte $C6, $00  ; row 0
.byte $C6, $00  ; row 1
.byte $6C, $00  ; row 2
.byte $38, $00  ; row 3
.byte $6C, $00  ; row 4
.byte $C6, $00  ; row 5
.byte $C6, $00  ; row 6
.byte $00, $00  ; row 7

; Character 89: Y
.byte $66, $00  ; row 0
.byte $66, $00  ; row 1
.byte $66, $00  ; row 2
.byte $3C, $00  ; row 3
.byte $18, $00  ; row 4
.byte $18, $00  ; row 5
.byte $18, $00  ; row 6
.byte $00, $00  ; row 7

; Character 90: Z
.byte $FE, $00  ; row 0
.byte $06, $00  ; row 1
.byte $0C, $00  ; row 2
.byte $18, $00  ; row 3
.byte $30, $00  ; row 4
.byte $60, $00  ; row 5
.byte $FE, $00  ; row 6
.byte $00, $00  ; row 7

; Character 91: [
.byte $3C, $00  ; row 0
.byte $30, $00  ; row 1
.byte $30, $00  ; row 2
.byte $30, $00  ; row 3
.byte $30, $00  ; row 4
.byte $30, $00  ; row 5
.byte $3C, $00  ; row 6
.byte $00, $00  ; row 7

; Character 92: backslash
.byte $C0, $00  ; row 0
.byte $60, $00  ; row 1
.byte $30, $00  ; row 2
.byte $18, $00  ; row 3
.byte $0C, $00  ; row 4
.byte $06, $00  ; row 5
.byte $02, $00  ; row 6
.byte $00, $00  ; row 7

; Character 93: ]
.byte $3C, $00  ; row 0
.byte $0C, $00  ; row 1
.byte $0C, $00  ; row 2
.byte $0C, $00  ; row 3
.byte $0C, $00  ; row 4
.byte $0C, $00  ; row 5
.byte $3C, $00  ; row 6
.byte $00, $00  ; row 7

; Character 94: ^
.byte $10, $00  ; row 0
.byte $38, $00  ; row 1
.byte $6C, $00  ; row 2
.byte $C6, $00  ; row 3
.byte $00, $00  ; row 4
.byte $00, $00  ; row 5
.byte $00, $00  ; row 6
.byte $00, $00  ; row 7

; Character 95: _
.byte $00, $00  ; row 0
.byte $00, $00  ; row 1
.byte $00, $00  ; row 2
.byte $00, $00  ; row 3
.byte $00, $00  ; row 4
.byte $00, $00  ; row 5
.byte $00, $00  ; row 6
.byte $FE, $00  ; row 7

font_data_end:
font_data_size = font_data_end - font_data
