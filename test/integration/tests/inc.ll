; INTEGRATION-TEST
; EXPECT: 43

target triple = "w65816-unknown-none"

define i16 @test_main() {
  ; 42 + 1 = 43
  %result = add i16 42, 1
  ret i16 %result
}
