; INTEGRATION-TEST
; EXPECT: 85

target triple = "w65816-unknown-none"

; 0xFF ^ 0xAA = 0x55 = 85
define i16 @test_main() {
  %result = xor i16 255, 170
  ret i16 %result
}
