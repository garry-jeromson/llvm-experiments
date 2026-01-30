; INTEGRATION-TEST
; EXPECT: 65535

target triple = "w65816-unknown-none"

; Maximum 16-bit value
define i16 @test_main() {
  ret i16 65535
}
