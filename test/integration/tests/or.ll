; INTEGRATION-TEST
; EXPECT: 255

target triple = "w65816-unknown-none"

; 0xF0 | 0x0F = 0xFF = 255
define i16 @test_main() {
  %result = or i16 240, 15
  ret i16 %result
}
