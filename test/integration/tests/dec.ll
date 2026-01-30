; INTEGRATION-TEST
; EXPECT: 99

target triple = "w65816-unknown-none"

define i16 @test_main() {
  ; 100 - 1 = 99
  %result = sub i16 100, 1
  ret i16 %result
}
