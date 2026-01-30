; INTEGRATION-TEST
; EXPECT: 4

target triple = "w65816-unknown-none"

; 256 >> 6 = 4
define i16 @test_main() {
  %result = lshr i16 256, 6
  ret i16 %result
}
