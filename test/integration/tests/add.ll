; INTEGRATION-TEST
; EXPECT: 42

target triple = "w65816-unknown-none"

define i16 @test_main() {
  %result = add i16 20, 22
  ret i16 %result
}
