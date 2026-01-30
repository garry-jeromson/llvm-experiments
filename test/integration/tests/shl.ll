; INTEGRATION-TEST
; EXPECT: 64

target triple = "w65816-unknown-none"

; 1 << 6 = 64
define i16 @test_main() {
  %result = shl i16 1, 6
  ret i16 %result
}
