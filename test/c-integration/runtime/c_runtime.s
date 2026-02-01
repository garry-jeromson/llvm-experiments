; C Integration Test Runtime Library
; ==================================
; Minimal runtime library for W65816 C integration tests.
; Provides arithmetic functions called by LLVM-generated code.
;
; Build with:
;   ca65 --cpu 65816 -o c_runtime.o c_runtime.s
;   ld65 -C c_runtime.cfg -o c_runtime.bin c_runtime.o
;
; Symbol offsets (must match RUNTIME_SYMBOLS in run_tests.py):
;   __mulhi3:  0x00
;   __udivhi3: 0x17
;   __divhi3:  0x3B
;   __umodhi3: 0x65
;   __modhi3:  0x82

.p816
.smart
.a16
.i16

.segment "CODE"

; Temporary storage in zero page
_tmp0 = $E0
_tmp1 = $E2
_tmp2 = $E4

.export __mulhi3
.export __divhi3
.export __udivhi3
.export __modhi3
.export __umodhi3

; ===========================================================================
; __mulhi3 - Unsigned 16-bit multiplication
; ===========================================================================
; Input:  A = multiplicand, X = multiplier
; Output: A = product (low 16 bits)
; Offset: 0x00
__mulhi3:
        sta _tmp0
        stx _tmp1
        lda #0
        ldy #16
@loop:
        lsr _tmp1
        bcc @skip
        clc
        adc _tmp0
@skip:
        asl _tmp0
        dey
        bne @loop
        rts

; ===========================================================================
; __udivhi3 - Unsigned 16-bit division
; ===========================================================================
; Input:  A = dividend, X = divisor
; Output: A = quotient
; Offset: 0x17
__udivhi3:
        cpx #0
        beq @div_zero
        sta _tmp0
        stx _tmp1
        lda #0
        ldy #16
@loop:
        asl _tmp0
        rol a
        cmp _tmp1
        bcc @skip
        sbc _tmp1
        inc _tmp0
@skip:
        dey
        bne @loop
        lda _tmp0
        rts
@div_zero:
        lda #$FFFF
        rts

; ===========================================================================
; __divhi3 - Signed 16-bit division
; ===========================================================================
; Input:  A = dividend, X = divisor
; Output: A = quotient
; Offset: 0x3B
__divhi3:
        stz _tmp2
        cmp #$8000
        bcc @pos_dividend
        eor #$FFFF
        inc a
        inc _tmp2
@pos_dividend:
        sta _tmp0
        txa
        cmp #$8000
        bcc @pos_divisor
        eor #$FFFF
        inc a
        inc _tmp2
@pos_divisor:
        tax
        lda _tmp0
        jsr __udivhi3
        lsr _tmp2
        bcc @done
        eor #$FFFF
        inc a
@done:
        rts

; ===========================================================================
; __umodhi3 - Unsigned 16-bit modulo
; ===========================================================================
; Input:  A = dividend, X = divisor
; Output: A = remainder
; Offset: 0x65
__umodhi3:
        cpx #0
        beq @div_zero
        sta _tmp0
        stx _tmp1
        lda #0
        ldy #16
@loop:
        asl _tmp0
        rol a
        cmp _tmp1
        bcc @skip
        sbc _tmp1
@skip:
        dey
        bne @loop
        rts
@div_zero:
        rts

; ===========================================================================
; __modhi3 - Signed 16-bit modulo
; ===========================================================================
; Input:  A = dividend, X = divisor
; Output: A = remainder
; Offset: 0x82
__modhi3:
        stz _tmp2
        cmp #$8000
        bcc @pos_dividend
        eor #$FFFF
        inc a
        inc _tmp2
@pos_dividend:
        sta _tmp0
        txa
        cmp #$8000
        bcc @pos_divisor
        eor #$FFFF
        inc a
@pos_divisor:
        tax
        lda _tmp0
        jsr __umodhi3
        lsr _tmp2
        bcc @done
        eor #$FFFF
        inc a
@done:
        rts
