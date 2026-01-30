; INTEGRATION-TEST
; EXPECT: 65493

target triple = "w65816-unknown-none"

define i16 @negate(i16 %x) {
  %result = sub i16 0, %x
  ret i16 %result
}

; negate(43) = -43 as unsigned 16-bit = 65493
define i16 @test_main() {
  %result = call i16 @negate(i16 43)
  ret i16 %result
}
