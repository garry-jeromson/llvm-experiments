; INTEGRATION-TEST
; EXPECT: 58

target triple = "w65816-unknown-none"

define i16 @test_main() {
  %result = sub i16 100, 42
  ret i16 %result
}
