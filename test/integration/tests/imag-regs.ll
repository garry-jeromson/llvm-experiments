; INTEGRATION-TEST
; EXPECT: 150

target triple = "w65816-unknown-none"

; Function that needs more than 3 registers to hold intermediate values
; This tests that imaginary registers work correctly at runtime
define i16 @test_main() {
entry:
  %a = add i16 10, 20    ; 30
  %b = add i16 30, 40    ; 70
  %c = add i16 %a, %b    ; 100
  %d = add i16 25, 25    ; 50
  %result = add i16 %c, %d  ; 150
  ret i16 %result
}
