; INTEGRATION-TEST
; EXPECT: 50

target datalayout = "e-m:e-p:16:16-i8:8-i16:16-n8:16-S16"
target triple = "w65816-unknown-none"

; Test SUB operations with register pressure requiring imaginary registers
; (100 - 30) + (90 - 10) = 70 + 80 = 150 - 100 = 50
define i16 @test_main() {
entry:
  %a = add i16 100, 0
  %b = add i16 30, 0
  %c = add i16 90, 0
  %d = add i16 10, 0
  ; Keep all four values live
  %diff1 = sub i16 %a, %b   ; 70
  %diff2 = sub i16 %c, %d   ; 80
  %sum = add i16 %diff1, %diff2  ; 150
  %result = sub i16 %sum, %a     ; 50
  ret i16 %result
}
