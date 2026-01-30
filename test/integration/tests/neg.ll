; INTEGRATION-TEST
; EXPECT: 65493

target triple = "w65816-unknown-none"

; -43 as unsigned 16-bit = 65493
define i16 @test_main() {
  %result = sub i16 0, 43
  ret i16 %result
}
