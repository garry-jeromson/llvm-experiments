; INTEGRATION-TEST
; EXPECT: 0

target triple = "w65816-unknown-none"

; Return zero
define i16 @test_main() {
  ret i16 0
}
