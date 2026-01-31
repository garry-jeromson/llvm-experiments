; INTEGRATION-TEST
; EXPECT: 42

target triple = "w65816-unknown-none"

; Test that double negation produces correct result: -(-(x)) == x
define i16 @double_negate(i16 %x) {
  %neg1 = sub i16 0, %x
  %neg2 = sub i16 0, %neg1
  ret i16 %neg2
}

; -(-(42)) should equal 42
define i16 @test_main() {
  %result = call i16 @double_negate(i16 42)
  ret i16 %result
}
