; INTEGRATION-TEST
; EXPECT: 100

target datalayout = "e-m:e-p:16:16-i8:8-i16:16-n8:16-S16"
target triple = "w65816-unknown-none"

; Test ADD operations with register pressure requiring imaginary registers
; 10 + 20 + 30 + 40 = 100
define i16 @test_main() {
entry:
  %a = add i16 10, 0
  %b = add i16 20, 0
  %c = add i16 30, 0
  %d = add i16 40, 0
  ; Keep all four values live
  %sum1 = add i16 %a, %b   ; 30
  %sum2 = add i16 %c, %d   ; 70
  %result = add i16 %sum1, %sum2  ; 100
  ret i16 %result
}
