; INTEGRATION-TEST
; EXPECT: 65532

target triple = "w65816-unknown-none"

; -16 >> 2 = -4 (sign extends)
; -4 as unsigned 16-bit = 65532
define i16 @test_main() {
  %result = ashr i16 -16, 2
  ret i16 %result
}
