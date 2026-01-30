; INTEGRATION-TEST
; EXPECT: 170

target triple = "w65816-unknown-none"

; 0xFFAA & 0x00FF = 0x00AA = 170
define i16 @test_main() {
  %result = and i16 65450, 255
  ret i16 %result
}
