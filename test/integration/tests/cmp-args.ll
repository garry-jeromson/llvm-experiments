; INTEGRATION-TEST
; EXPECT: 1

target triple = "w65816-unknown-none"

; Test comparison with function arguments
; This exercises the CMP16rr path
define i16 @is_less(i16 %a, i16 %b) {
entry:
  %cmp = icmp slt i16 %a, %b
  br i1 %cmp, label %then, label %else
then:
  ret i16 1
else:
  ret i16 0
}

; is_less(10, 20) should be 1
define i16 @test_main() {
  %result = call i16 @is_less(i16 10, i16 20)
  ret i16 %result
}
