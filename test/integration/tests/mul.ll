; INTEGRATION-TEST
; EXPECT: 56
; SKIP: Multiply requires runtime library (not yet linked)

target triple = "w65816-unknown-none"

; 7 * 8 = 56
define i16 @test_main() {
  %result = mul i16 7, 8
  ret i16 %result
}
